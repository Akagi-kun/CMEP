import re
import json
from pathlib import Path

def parse_vcxproj(project_file):
    with open(project_file, 'r') as file:
        data = file.read()
    
    # Extract relevant information from the .vcxproj file
    # This is a simplified example; you may need to adjust the parsing logic
    includes = []
    defines = []
    files = []
    
    # Find include directories
    include_lines = re.findall(r'<AdditionalIncludeDirectories>(.*?)</AdditionalIncludeDirectories>', data)
    for line in include_lines:
        includes.extend(line.split(';'))
    
    # Find preprocessor definitions
    define_lines = re.findall(r'<PreprocessorDefinitions>(.*?)</PreprocessorDefinitions>', data)
    for line in define_lines:
        defines.extend(line.split(';'))
    
    # Find source files
    file_lines = re.findall(r'<ClCompile Include="(.*?)"', data)
    files.extend(file_lines)
    
    return includes, defines, files

def generate_compile_commands(vs_project_path, output_path):
    compile_commands = []
    
    for project_file in Path(vs_project_path).rglob('*.vcxproj'):
        includes, defines, files = parse_vcxproj(project_file)
        print(f"Parsing {project_file}")

        for file in files:
            command = {
                'directory': str(Path(project_file).parent),
                'command': f'cl /I{" /I".join(includes)} /D{" /D".join(defines)} /c {file}',
                'file': file
            }
            compile_commands.append(command)
    
    with open(output_path, 'w') as f:
        json.dump(compile_commands, f, indent=2)

if __name__ == '__main__':
    vs_project_path = 'build/'
    output_path = 'build/compile_commands.json'
    generate_compile_commands(vs_project_path, output_path)