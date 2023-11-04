import csv
from collections import Counter


# def find_delayed_pkt(csv_file_path, column_index1, column_index2):
#     # Read the CSV file
#     with open(csv_file_path, newline='', encoding='utf-8') as file:
#         reader = csv.reader(file)
#         next(reader, None)  # Skip the header row if it exists
#         # Collect all strings in the specified column
#         strings1 = [row[column_index1]
#                     for row in reader if len(row) > column_index1]
#         strings2 = [row[column_index2]
#                     for row in reader if len(row) > column_index2]

#         for sb in range(len(strings1)):
#             print(strings1[sb],strings2[sb])
#             if()
#             if (int)(strings1[sb]) < (int)(strings2[sb]):
#                 print(strings1)


def find_unique_strings(csv_file_path, column_index):
    # Read the CSV file
    with open(csv_file_path, newline='', encoding='utf-8') as file:
        reader = csv.reader(file)
        next(reader, None)  # Skip the header row if it exists
        # Collect all strings in the specified column
        strings = [row[column_index]
                   for row in reader if len(row) > column_index]

    # Count the occurrences of each string
    string_counts = Counter(strings)

    # Find and return the strings that only show up once
    unique_strings = [string for string,
                      count in string_counts.items() if count == 1]
    return unique_strings


# Specify the path to your CSV file
csv_file_path = "..//simulation/log.txt"
# Specify the index of the column you're interested in (e.g., 0 for the first column)
column_index = 1

unique_strings = find_unique_strings(csv_file_path, column_index)
for unique_string in unique_strings[12: 99]:
    print(unique_string)

# find_delayed_pkt(csv_file_path, 2, 3)
