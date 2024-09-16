#include <vector>
#include <string>

class user
{
public:
    int id;
    std::string nickName;
    int idRoom;

    bool operator==(const user &other) const
    {
        return this->id == other.id;
    }
    bool operator!=(const user &other)
    {
        return this->id != other.id;
    }

    user(int id_, std::string nickName_) : id(id_), nickName(nickName_), idRoom(0) {}
};




int main() {}