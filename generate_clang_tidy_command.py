import os

build_str = "clang-tidy --verify-config && clang-tidy"

def handle_files(root, files):
	global build_str
	if len(files) > 0:
		for file in files:
			build_str += " " + os.path.join(root, file)
		#print(files)

def walk_path(dir):
	for root, dirs, files in os.walk(dir):
		handle_files(root, files)

		for dir in dirs:
			walk_path(dir)

def main():
	# Walk every library
	walk_path("EngineCore/src")
	#walk_path("EngineLogging/src")

	print("Generated the following clang-tidy command:\n")
	print(build_str)

if __name__ == "__main__":
	main()