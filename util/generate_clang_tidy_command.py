import os
import argparse
from pathlib import Path

build_str = "clang-tidy --verify-config && clang-tidy"

def handle_files(root, files):
	global build_str
	if len(files) > 0:
		for file in files:
			build_str += " " + str(Path(os.path.join(root, file)))
		#print(files)

def walk_path(dir):
	for root, dirs, files in os.walk(dir):
		handle_files(Path(root), files)

		for dir in dirs:
			walk_path(dir)

def main():
	parser = argparse.ArgumentParser(prog="clang-tidy command generator", description="generates a clang-tidy command with every source file of a specific folder")
	parser.add_argument("folder")
	args = parser.parse_args()

	# Walk every library
	walk_path(args.folder + "/src")

	print("Generated the following clang-tidy command:\n")
	print(build_str)

if __name__ == "__main__":
	main()