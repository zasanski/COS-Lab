#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "argparse.hpp"

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("program_name");

  program.add_argument("-i", "--input").default_value(std::string("-")).required().help("specify the input file.");
  program.add_argument("-o", "--output").default_value(std::string("-")).required().help("specify the output file.");

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  auto input = program.get<std::string>("input");
  std::cout << "INPUT: " << input << std::endl;
  auto output = program.get<std::string>("output");
  std::cout << "OUTPUT: " << output << std::endl;

  std::ifstream file_in;
  std::ofstream file_out;

  file_in.open(input, std::ios::binary);
  file_out.open(output, std::ios::binary);

  if (!file_in.is_open()) {
    std::cerr << "Error: Could not open input file: " << input << std::endl;
    return 1;
  }

  if (!file_out.is_open()) {
    std::cerr << "Error: Could not open output file: " << output << std::endl;
    file_in.close();
    return 1;
  }

  const size_t buffer_size = BUFSIZ;
  std::vector<char> buffer(buffer_size);

  while (file_in.read(buffer.data(), buffer_size)) {
    file_out.write(buffer.data(), file_in.gcount());
  }

  file_out.write(buffer.data(), file_in.gcount());

  file_in.close();
  file_out.close();

  return 0;
}
