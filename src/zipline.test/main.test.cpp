#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <timber/timber>

namespace fs = std::filesystem;

namespace {
    const auto log_path = fs::temp_directory_path() / "zipline.test.log";
    auto log_file = std::ofstream(log_path);

    auto file_logger(const timber::log& l) noexcept -> void {
        log_file
            << "[" << l.log_level << "] "
            << l.stream.str()
            << std::endl;
    }
}

auto main(int argc, char** argv) -> int {
    timber::log_handler = &file_logger;

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
