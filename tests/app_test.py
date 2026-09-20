"""Exercise monthly input and rent/laundry accounting for both models."""
import csv
from pathlib import Path
import re
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[1]


def run_app(model, rent, laundry, daily=(10000, 2000, 3000), budget=18000,
            weekly_daily=None):
    values = [rent, *laundry]
    values.extend(daily * 7 if weekly_daily is None else weekly_daily)
    values.extend((budget, 3000000))
    return subprocess.run(
        [str(ROOT / 'build/expense_tracker'), model],
        input='\n'.join(map(str, values)) + '\n', text=True,
        capture_output=True, cwd=ROOT, check=True,
    ).stdout


class MonthlyExpenseTest(unittest.TestCase):
    def test_csv_totals_without_rent(self):
        for path in (ROOT / 'data').glob('*.csv'):
            with path.open(newline='') as source:
                reader = csv.DictReader(source)
                self.assertNotIn('Rent', reader.fieldnames)
                for row in reader:
                    total = sum(float(row[key]) for key in
                                ('Food', 'Drinks', 'Transportation', 'Laundry'))
                    self.assertEqual(float(row['Total_Expense']), total)
                    expected = ('OVER_BUDGET' if total > float(row['Daily_Budget'])
                                else 'WITHIN_BUDGET')
                    self.assertEqual(row['Budget_Status'], expected)

    def test_weekly_laundry_and_monthly_rent(self):
        for model in ('linear', 'knn'):
            with self.subTest(model=model):
                output = run_app(model, 1200000, (7000, 14000, 21000, 28000))
                self.assertEqual(output.count('Rent 1 bulan (4 minggu): Rp '), 1)
                self.assertEqual(output.count('Laundry minggu ini: Rp '), 4)
                for category in ('Food', 'Drinks', 'Transportation'):
                    self.assertEqual(output.count(f'  {category}: Rp '), 7)
                history = output.split('2. EXPENSE HISTORY\n')[1].split('3. EXPENSE STATISTICS')[0]
                allocations = re.findall(r' \| Laundry: (\d+\.\d+) \| Total harian', history)
                self.assertEqual(allocations, [f'{week * 1000:.2f}'
                                               for week in range(1, 5) for _ in range(7)])
                for week, total in enumerate((412000, 419000, 426000, 433000), 1):
                    self.assertIn(f'Total minggu {week} (termasuk Rent): Rp {total}\n', output)
                self.assertIn('Total 4 minggu (termasuk Rent): Rp 1690000\n', output)
                self.assertIn('Sisa budget aktual 4 minggu: Rp 1310000\n', output)
                actual = re.findall(r'Aktual: (\d+) \((\w+)\)', output)
                self.assertEqual(actual, [(str(15000 + week * 1000),
                                          'OVER_BUDGET' if week == 4 else 'WITHIN_BUDGET')
                                         for week in range(1, 5) for _ in range(7)])

    def test_daily_pattern_reused_with_separate_weekly_laundry(self):
        pattern = [(1000 * day, 200 * day, 300 * day) for day in range(1, 8)]
        for model in ('linear', 'knn'):
            with self.subTest(model=model):
                output = run_app(model, 0, (7000, 14000, 21000, 28000),
                                 weekly_daily=[value for day in pattern for value in day])
                history = output.split('2. EXPENSE HISTORY\n')[1].split('3. EXPENSE STATISTICS')[0]
                rows = re.findall(r' \| Food: ([\d.]+) \| Drinks: ([\d.]+)'
                                  r' \| Transportation: ([\d.]+) \| Laundry: ([\d.]+)', history)
                expected = [tuple(map(float, (*day, week * 1000)))
                            for week in range(1, 5) for day in pattern]
                self.assertEqual([tuple(map(float, row)) for row in rows], expected)

    def test_rent_does_not_change_daily_predictions_or_status(self):
        for model in ('linear', 'knn'):
            with self.subTest(model=model):
                without = run_app(model, 0, (7000, 14000, 21000, 28000))
                with_rent = run_app(model, 1200000, (7000, 14000, 21000, 28000))
                daily = r'^.* \| Aktual: .* \| Forecast: .*'
                self.assertEqual(re.findall(daily, without, re.M),
                                 re.findall(daily, with_rent, re.M))
                weekly = r'Total prediksi minggu \d+ \(termasuk Rent\): Rp (\d+)'
                before = list(map(int, re.findall(weekly, without)))
                after = list(map(int, re.findall(weekly, with_rent)))
                self.assertEqual(len(before), 4)
                self.assertEqual([b - a for a, b in zip(before, after)], [300000] * 4)
                monthly = r'Total prediksi 4 minggu \(termasuk Rent\): Rp (\d+)'
                self.assertEqual(int(re.search(monthly, with_rent)[1]) -
                                 int(re.search(monthly, without)[1]), 1200000)

    def test_zero_and_fractional_laundry(self):
        for model in ('linear', 'knn'):
            with self.subTest(model=model):
                output = run_app(model, 0, (0, 10000, 0, 0), daily=(0, 0, 0))
                self.assertIn('Total minggu 2 (termasuk Rent): Rp 10000\n', output)
                self.assertIn('Total 4 minggu (termasuk Rent): Rp 10000\n', output)
                self.assertEqual(output.count(' | Laundry: 1428.57 | Total harian'), 7)


if __name__ == '__main__':
    unittest.main()
