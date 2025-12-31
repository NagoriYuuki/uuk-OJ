#include "base_entity.hpp"

#include <string>

class User : public Entity
{
public:
    User() = default;
    User(const int id,
         const std::string &username,
         const std::string &password_hash)
        : id(id),
          username(username),
          password_hash(password_hash) {}
    ~User() override = default;
    int id;
    std::string username;
    std::string password_hash;
    std::string created_time;
};