#include <string>
#include <chrono>
#include <filesystem>

std::string asString(const std::filesystem::perms& pm) {
    using perms = std::filesystem::perms;
    std::string s;
    s.resize(9);
    s[0] = (pm & perms::owner_read)   != perms::none ? 'r' : '-';
    s[1] = (pm & perms::owner_write)  != perms::none ? 'w' : '-';
    s[2] = (pm & perms::owner_exec)   != perms::none ? 'x' : '-';
    s[3] = (pm & perms::group_read)   != perms::none ? 'r' : '-';
    s[4] = (pm & perms::group_write)  != perms::none ? 'w' : '-';
    s[5] = (pm & perms::group_exec)   != perms::none ? 'x' : '-';
    s[6] = (pm & perms::others_read)  != perms::none ? 'r' : '-';
    s[7] = (pm & perms::others_write) != perms::none ? 'w' : '-';
    s[8] = (pm & perms::others_exec)  != perms::none ? 'x' : '-';
    return s;
}