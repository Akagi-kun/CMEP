import csv
import png
import re
import math

def process_csv():
	valuemap1: list[list[int]] = []
	valuemap2: list[list[int]] = []
	valuemap3: list[list[int]] = []

	dim = 300
	for i in range(dim):
		valuemap1.append([0] * dim)
		valuemap2.append([0] * dim)
		valuemap3.append([0] * dim)

	#print(valuemap)

	line_count = 0

	max_x = 0; min_x = 200
	max_z = 0; min_z = 200
	
	with open("test.csv") as csv_file:
		csv_reader = csv.reader(csv_file, delimiter=",")

		for row in csv_reader:
			if line_count == 0:
				print(f"Column names are {', '.join(row)}")
			else:
				x = int(row[0])
				z = int(row[1])

				val1 = math.floor((float(row[2]) + 1) * 5)
				val2 = math.floor((float(row[3]) + 1) * 4)

				if x == 3 or x == 4:
					print(f"\t{x}/{z}\t has values: {val1}/{val2}")

				max_x = max(max_x, x)
				min_x = min(min_x, x)

				max_z = max(max_z, z)
				min_z = min(min_z, z)

				valuemap1[x][z] = val1
				valuemap2[x][z] = val2
				valuemap3[x][z] = math.floor((val1 / 2) + (val2 / 2))
			line_count += 1

	print(f"Processed {line_count} lines.")
	print(max_x, min_x, max_z, min_z)

	return valuemap1, valuemap2, valuemap3

def generate_csv():
	with open("test.csv", "w") as writefile:
		writefile.write("x,y,noise_adjust,noise2\n")
		with open("build/latest.log", "r") as logfile:
			for line in logfile:
				processed_line = line.strip()
				processed_line = re.sub(r"^\[[0-9:]+ ", "", processed_line)
				processed_line = re.sub(r"^engine [DBGINFO]+", "", processed_line)
				processed_line = re.sub(r"^\] ", "", processed_line)
				if processed_line.startswith("[") and processed_line.endswith("]"):
					processed_line = processed_line.strip("[]")
					processed_line = re.sub(r"\s", "", processed_line)
					writefile.write(f"{processed_line}\n")


def main():
	generate_csv()

	map1, map2, map3 = process_csv()

	png.from_array(map1, 'L').save("noise1.png")
	png.from_array(map2, 'L').save("noise2.png")
	png.from_array(map3, 'L').save("noise3.png")

if __name__ == "__main__":
	main()