import re
import json
import os
import subprocess
from pathlib import Path

def parse_vcxproj(project_file):
	with open(project_file, 'r') as file:
		data = file.read()

	# Extract relevant information from the .vcxproj file
	# This is a simplified example; you may need to adjust the parsing logic
	includes = []
	defines = []
	files = []
	warning_level = 0

	warning_level_lines = re.findall(r'<WarningLevel>Level(.*?)</WarningLevel>', data)
	for line in warning_level_lines:
		warning_level = int(line)

	# Find include directories
	include_lines = re.findall(r'<AdditionalIncludeDirectories>(.*?)</AdditionalIncludeDirectories>', data)
	for line in include_lines:
		line_includes = [x for x in line.split(';') if not x.startswith('%') ]

		includes.extend(line_includes)

	# Find preprocessor definitions
	define_lines = re.findall(r'<PreprocessorDefinitions>(.*?)</PreprocessorDefinitions>', data)
	for line in define_lines:
		line_defines = [x for x in line.split(';') if not x.startswith('%') ]

		defines.extend(line_defines)

	# Find source files
	file_lines = re.findall(r'<ClCompile Include="(.*?)"', data)
	files.extend(file_lines)

	return list(set(includes)), list(set(defines)), files, warning_level

def generate_compile_commands(vs_project_path, output_path, root_path, default_includes : list):
	compile_commands = []

	default_includes_as_str: list[str] = [str(include) for include in default_includes]

	files_listing: list[str] = []

	print(f"- Generating compile_commands.json")
	for project_file in Path(vs_project_path).rglob('*.vcxproj'):
		includes, defines, files, warning_level = parse_vcxproj(project_file)
		print(f"  Parsing {project_file}")

		processed_file: str = str()
		processed_includes: list[str] = []

		processed_includes.extend(default_includes_as_str)

		for include in includes:
			include_as_path = Path(include)

			if "VulkanSDK" not in str(include_as_path):
				processed_includes.append(str(include_as_path.resolve().relative_to(root_path)))
			else:
				processed_includes.append(str(include_as_path))

		arguments: list = ["cl", "/EHsc", "/std:c++17", "/showIncludes", f"/W{warning_level}"]

		for include in processed_includes:
			arguments.append("/I" + include)

		#for define in defines:
		#	arguments.append("/D" + define)

		arguments.extend(["/D_DEBUG", "/DDEBUG=1", "/DWIN32", "/D_WINDOWS", "/DCMAKE_INTDIR=\"Debug\""])

		for file in files:
			if file.startswith("CMake"):
				processed_file = str(Path(file))
			else:
				processed_file = str(Path(file).relative_to(root_path))

			files_listing.append(processed_file + "\n")

			command = {
				'directory': str(Path(root_path)),
				#'command': f'cl /showIncludes /W{warning_level} /I{" /I".join(processed_includes)} /D{" /D".join(defines)} /c {file}',
				'arguments': arguments + ["/c", file],
				'file': processed_file
			}
			compile_commands.append(command)

	print(f"- Writing compile_commands.json")
	with open(output_path + "compile_commands.json", 'w') as f:
		json.dump(compile_commands, f, indent=2)

	print(f"- Writing source_files.txt")
	with open(output_path + "source_files.txt", "w") as f:
		f.writelines(files_listing)

def get_default_includes():
	vswhere_path = os.path.expandvars("%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere.exe")

	print("- Running vswhere.exe")
	lines: list = subprocess.check_output(vswhere_path).decode().splitlines()

	install_path: Path = ""
	for line in lines:
		if line.startswith("installationPath"):
			install_path = Path(line[17:].strip())
			break

	print(f"- VS Installed at '{str(install_path)}'")

	tool_path: Path = install_path.joinpath("VC", "Auxiliary", "Build")
	vcvars_path: str = (str(tool_path) + "\\vcvarsall.bat")

	print(f"- vcvarsall.bat at '{ vcvars_path }'")

	p = subprocess.Popen(['cmd', '/v:on', '/q', '/c', vcvars_path, "x64", '&echo(!INCLUDE!'], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, _ = p.communicate()
	lines: list[str] = stdout.decode().splitlines()

	paths: list[Path] = [Path(line.strip()) for line in lines[5:][0].split(";")]

	return paths

if __name__ == '__main__':
	vs_project_path = 'build/'
	output_path = './'
	root_path = Path().absolute()

	print(f"Generating cmake-tidy files at '{output_path}' from projects at '{vs_project_path}'")
	print(f"- Root is '{str(root_path)}'")

	default_includes = get_default_includes()
	generate_compile_commands(vs_project_path, output_path, root_path, default_includes)