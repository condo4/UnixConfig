#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>
#include <vector>

/*
 * Source of configuration:
 * /etc/<id>.conf
 * /etc/id.conf.d/*.conf
 * ~/.config/id.conf
 */
class Config
{
public:
    Config(std::string id);
    const std::string &operator[](const std::string&) const;
    const std::vector<std::string>& keys() const;
    const std::vector<std::string>& arrays() const;
    const std::vector<std::string>& array(const std::string &id);

private:
    std::string m_id;
    std::map<std::string, std::string> m_map;
    std::map<std::string, std::vector<std::string>> m_map_arrays;
    std::string m_empty;
    std::vector<std::string> m_empty_array;
    mutable std::vector<std::string> m_keys;
    mutable std::vector<std::string> m_keys_arrays;

    int _parsePath(std::string path);
    int _parseFile(std::string file);
};

#endif // CONFIG_H
