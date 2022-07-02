/**
 * @file Files.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Lots of utilities about the filesystem
 * @version 0.1
 * @date 2021-11-25
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef INCLUDE_ARK_FILES_HPP
#define INCLUDE_ARK_FILES_HPP

#include <string>
#include <fstream>
#include <streambuf>
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
        return std::string(
            (std::istreambuf_iterator<char>(f)),
            std::istreambuf_iterator<char>());
    }

    /**
     * @brief Get the directory from a path
     * 
     * @param path 
     * @return std::string 
     */
    inline std::string getDirectoryFromPath(const std::string& path)
    {
        return (std::filesystem::path(path)).parent_path().string();
    }

    /**
     * @brief Get the filename from a path
     * 
     * @param path 
     * @return std::string 
     */
    inline std::string getFilenameFromPath(const std::string& path)
    {
        return (std::filesystem::path(path)).filename().string();
    }

    /**
     * @brief Get the canonical relative path from a path
     * 
     * @param path 
     * @return std::string 
     */
    inline std::string canonicalRelPath(const std::string& path)
    {
        return std::filesystem::relative(std::filesystem::path(path)).generic_string();
    }
}

#endif
