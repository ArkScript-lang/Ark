#include <Ark/Utils.hpp>

namespace Ark::Utils
{
    std::size_t levenshteinDistance(const std::string& str1, const std::string& str2)
    {
        const std::size_t str1_len = str1.size();
        const std::size_t str2_len = str2.size();
        std::vector edit_distances(str1_len + 1, std::vector<std::size_t>(str2_len + 1, 0));

        for (std::size_t i = 0; i < str1_len + 1; i++)
            edit_distances[i][0] = i;

        for (std::size_t j = 0; j < str2_len + 1; j++)
            edit_distances[0][j] = j;

        for (std::size_t i = 1; i < str1_len + 1; i++)
        {
            for (std::size_t j = 1; j < str2_len + 1; j++)
            {
                const std::size_t indicator = str1[i - 1] == str2[j - 1] ? 0 : 1;
                edit_distances[i][j] = std::min({
                    edit_distances[i - 1][j] + 1,             // deletion
                    edit_distances[i][j - 1] + 1,             // insertion
                    edit_distances[i - 1][j - 1] + indicator  // substitution
                });
            }
        }

        return edit_distances[str1_len][str2_len];
    }
}
