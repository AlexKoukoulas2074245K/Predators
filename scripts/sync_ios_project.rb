#import the xcodeproj ruby gem
require 'xcodeproj'

#define the path to your .xcodeproj file
project_path = '../prebuilt_ios/PredatorsIOS/PredatorsIOS.xcodeproj'

#open the xcode project
project = Xcodeproj::Project.open(project_path)

#find all relevant source files
files = Dir.glob(["../source_common/**/*.cpp", "../source_common/**/*.h", "../source_ios/**/*.cpp", "../source_ios/**/*.h", "../source_ios/**/*.m", "../source_ios/**/*.mm"]).select{|file| !file.include? "imgui" and !file.include? "main.cpp"}

for f in files do
  f = "../" + f 
  file_reference_exists = project.files.find { |file| file.full_path.to_s == f }
  
  # move on to the next file if it does
  next if file_reference_exists

  # assign the main group to a variable
  group = project.main_group

  # represent the file path as an array of subfolders
  split_file_path = ("PredatorsIOS/" + f[6..-1]).split('/')

  # get the file name and remove it from the array
  file_name = split_file_path.pop

  split_file_path.each do |subfolder|
    if group[subfolder]
      # move on to the next subfolder if the current one already has a reference
      group = group[subfolder]
    else
      # create a new Xcode group if the current subfolder doesn't have a reference
      group = group.new_group(subfolder)

      # for some reason Xcodeproj sets the full path for the newly created group,
      # whereas Xcode expects it to be just a folder name ¯\_(ツ)_/¯
      group.path = subfolder
    end
  end

  # create a new file reference
  file_reference = group.new_file(f)

  # once again, replace the full path in file reference to a file name
  file_reference.path = file_name

  # add the file reference to the first (main) target's `Compile Sources` build phase
  # even though the main target will work for most cases, you might want to replace it with a specific one
  project.targets.first.source_build_phase.add_file_reference(file_reference)

  puts "Copied over #{f}"
end

project.save
