/**
 * @file Files.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Lots of utilities about the filesystem
 * @version 0.3
 * @date 2024-07-09
 *
 * @copyright Copyright (c) 2021-2024
 *
 */

#ifndef INCLUDE_ARK_FILES_HPP
#define INCLUDE_ARK_FILES_HPP

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

namespace Ark::Utils
{
    /**
     * @brief Checks if a file exists
     *
     * @param name the file name
     * @return true on success
     * @return false on failure
     */
    inline bool fileExists(const std::string& name) noexcept
    {
        try
        {
            return std::filesystem::exists(std::filesystem::path(name));
        }
        catch (const std::filesystem::filesystem_error&)
        {
            // if we met an error than we most likely fed an invalid path
            return false;
        }
    }

    /**
     * @brief Helper to read a file
     *
     * @param name the file name
     * @return std::string
     */
    inline std::string readFile(const std::string& name)
    {
        std::ifstream f(name);
        // admitting the file exists
        return {
            (std::istreambuf_iterator<char>(f)),
            std::istreambuf_iterator<char>()
        };
    }

    /**
     * @brief Helper to read the bytes of a file
     * @param name filename
     * @return std::vector<uint8_t>
     */
    inline std::vector<uint8_t> readFileAsBytes(const std::string& name)
    {
        // admitting the file exists
        std::ifstream ifs(name, std::ios::binary | std::ios::ate);
        if (!ifs.good())
            return std::vector<uint8_t> {};

        const auto pos = ifs.tellg();
        // reserve appropriate number of bytes
        std::vector<char> temp(static_cast<std::size_t>(pos));
        ifs.seekg(0, std::ios::beg);
        ifs.read(&temp[0], pos);
        ifs.close();

        auto bytecode = std::vector<uint8_t>(static_cast<std::size_t>(pos));
        // TODO would it be faster to memcpy?
        for (std::size_t i = 0; i < static_cast<std::size_t>(pos); ++i)
            bytecode[i] = static_cast<uint8_t>(temp[i]);
        return bytecode;
    }

    /**
     * @brief Get the canonical relative path from a path
     *
     * @param path
     * @return std::string
     */
    inline std::string canonicalRelPath(const std::string& path)
    {
        return relative(std::filesystem::path(path)).generic_string();
    }
}

#endif
