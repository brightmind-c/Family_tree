#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <sstream>
#include <queue>

struct house; // Forward declaration

struct person {
    std::string name;
    house* birth_house;
    house* own_house;
    bool is_male; // true = ชาย, false = หญิง
    size_t year;

    person(std::string n, bool male, size_t y)
        : name(n), birth_house(nullptr), own_house(nullptr), is_male(male), year(y) {}
};

struct house {
    std::string house_name;
    person* pa;
    person* ma;
    std::vector<person*> kids;

    house(std::string h_name, person* father, person* mother)
        : house_name(h_name), pa(father), ma(mother) {
        if (pa) pa->own_house = this;
        if (ma) ma->own_house = this;
    }

    void add_child(person* child) {
        kids.push_back(child);
        child->birth_house = this;

        // จัดเรียงลูกตามปีเกิด (พี่ -> น้อง) อัตโนมัติ
        std::stable_sort(kids.begin(), kids.end(), [](const person* a, const person* b) {
            return a->year < b->year;
        });
    }
};

// ------------------------------------------------------------------
// ฟังก์ชันช่วยเก็บรวบรวมญาติทั้งหมดที่เชื่อมโยงถึงกัน (Graph Traversal / BFS)
// เพื่อเช็คว่า A และ B อยู่ใน Component เดียวกันหรือไม่
// ------------------------------------------------------------------
bool are_in_same_component(const person* start, const person* target) {
    if (!start || !target) return false;
    if (start == target) return true;

    std::unordered_set<const person*> visited;
    std::queue<const person*> q;

    q.push(start);
    visited.insert(start);

    while (!q.empty()) {
        const person* curr = q.front();
        q.pop();

        if (curr == target) return true;

        // รวบรวมเพื่อนบ้านทั้งหมดของ curr (พ่อ, แม่, คู่สมรส, ลูกๆ, พี่น้องในบ้านเกิด)
        std::vector<const person*> neighbors;

        if (curr->birth_house) {
            if (curr->birth_house->pa) neighbors.push_back(curr->birth_house->pa);
            if (curr->birth_house->ma) neighbors.push_back(curr->birth_house->ma);
            for (auto k : curr->birth_house->kids) neighbors.push_back(k);
        }

        if (curr->own_house) {
            if (curr->own_house->pa) neighbors.push_back(curr->own_house->pa);
            if (curr->own_house->ma) neighbors.push_back(curr->own_house->ma);
            for (auto k : curr->own_house->kids) neighbors.push_back(k);
        }

        for (auto next_p : neighbors) {
            if (next_p && visited.find(next_p) == visited.end()) {
                visited.insert(next_p);
                q.push(next_p);
            }
        }
    }

    return false;
}

// ------------------------------------------------------------------
// ฟังก์ชันคำนวณคำเรียกญาติ (ภาษาอังกฤษ)
// ------------------------------------------------------------------
std::string get_english_kinship(const person* A, const person* B) {
    if (!A || !B) return "Unknown";
    if (A == B) return "Self";

    house* A_birth = A->birth_house;
    house* A_own   = A->own_house;
    house* B_birth = B->birth_house;

    // 1. พ่อ / แม่
    if (A_birth) {
        if (A_birth->pa == B) return "Father";
        if (A_birth->ma == B) return "Mother";
    }

    // 2. ลูก (Son / Daughter)
    if (A_own) {
        auto& kids = A_own->kids;
        if (std::find(kids.begin(), kids.end(), B) != kids.end()) {
            return B->is_male ? "Son" : "Daughter";
        }
    }

    // 3. คู่สมรส (Husband / Wife)
    if (A_own) {
        if (A->is_male && A_own->ma == B) return "Wife";
        if (!A->is_male && A_own->pa == B) return "Husband";
    }

    // 4. พี่น้องท้องเดียวกัน (Brother / Sister)
    if (A_birth && B_birth && A_birth == B_birth) {
        if (B->year < A->year) {
            return B->is_male ? "Older Brother" : "Older Sister";
        } else if (B->year > A->year) {
            return B->is_male ? "Younger Brother" : "Younger Sister";
        } else {
            return B->is_male ? "Twin Brother" : "Twin Sister";
        }
    }

    // 5. ปู่ ย่า ตา ยาย (Grandfather / Grandmother)
    if (A_birth) {
        if (A_birth->pa && A_birth->pa->birth_house) {
            house* p_birth = A_birth->pa->birth_house;
            if (p_birth->pa == B) return "Paternal Grandfather";
            if (p_birth->ma == B) return "Paternal Grandmother";
        }
        if (A_birth->ma && A_birth->ma->birth_house) {
            house* m_birth = A_birth->ma->birth_house;
            if (m_birth->pa == B) return "Maternal Grandfather";
            if (m_birth->ma == B) return "Maternal Grandmother";
        }
    }

    // 6. ลุง ป้า น้า อา (Uncle / Aunt)
    if (A_birth && A_birth->pa && A_birth->pa->birth_house) {
        house* p_birth = A_birth->pa->birth_house;
        if (B_birth == p_birth) {
            person* dad = A_birth->pa;
            if (B->is_male) {
                return (B->year < dad->year) ? "Paternal Uncle (Older)" : "Paternal Uncle (Younger)";
            } else {
                return "Paternal Aunt";
            }
        }
    }
    if (A_birth && A_birth->ma && A_birth->ma->birth_house) {
        house* m_birth = A_birth->ma->birth_house;
        if (B_birth == m_birth) {
            person* mom = A_birth->ma;
            if (B->is_male) {
                return "Maternal Uncle";
            } else {
                return (B->year < mom->year) ? "Maternal Aunt (Older)" : "Maternal Aunt (Younger)";
            }
        }
    }

    // 7. ถ้ายังไม่มีคำเรียกเฉพาะ แต่เชื่อมโยงถึงกันในกราฟเดียวกัน
    if (are_in_same_component(A, B)) {
        return "Distant Relative";
    }

    // 8. ถ้าหลุดออกจากกราฟ (คนละตระกูล ไม่เชื่อมกันเลย)
    return "Not Related";
}

// ------------------------------------------------------------------
// Main Program & Command Parser
// ------------------------------------------------------------------
int main() {
    std::unordered_map<std::string, person*> people;
    std::unordered_map<std::string, house*> houses;

    std::cout << "=== Family Tree Interactive System (Updated) ===\n";
    std::cout << "Commands:\n";
    std::cout << "  person <name> <M/F> <year>\n";
    std::cout << "  house <house_name> <father> <mother>\n";
    std::cout << "  add_kids <house_name> <child1> <child2> ... (Add multiple kids)\n";
    std::cout << "  ask <personA> <personB>\n";
    std::cout << "  exit\n";
    std::cout << "--------------------------------------------------\n";

    std::string line;
    while (true) {
        std::cout << "\n> ";
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (cmd == "exit") {
            break;
        }
        else if (cmd == "person") {
            std::string name;
            char gender_char;
            size_t year;
            if (ss >> name >> gender_char >> year) {
                bool is_male = (gender_char == 'M' || gender_char == 'm');
                if (people.find(name) == people.end()) {
                    people[name] = new person(name, is_male, year);
                    std::cout << "[Success] Created person: " << name << "\n";
                } else {
                    std::cout << "[Error] Person '" << name << "' already exists!\n";
                }
            } else {
                std::cout << "[Usage] person <name> <M/F> <year>\n";
            }
        }
        else if (cmd == "house") {
            std::string h_name, f_name, m_name;
            if (ss >> h_name >> f_name >> m_name) {
                if (houses.find(h_name) != houses.end()) {
                    std::cout << "[Error] House '" << h_name << "' already exists!\n";
                    continue;
                }
                person* father = (f_name != "none" && people.count(f_name)) ? people[f_name] : nullptr;
                person* mother = (m_name != "none" && people.count(m_name)) ? people[m_name] : nullptr;

                houses[h_name] = new house(h_name, father, mother);
                std::cout << "[Success] Created house: " << h_name << "\n";
            } else {
                std::cout << "[Usage] house <house_name> <father_name> <mother_name>\n";
            }
        }
        else if (cmd == "add_kids") {
            std::string h_name, c_name;
            if (ss >> h_name) {
                if (houses.count(h_name)) {
                    int count = 0;
                    while (ss >> c_name) {
                        if (people.count(c_name)) {
                            houses[h_name]->add_child(people[c_name]);
                            count++;
                        } else {
                            std::cout << "[Warning] Person '" << c_name << "' not found, skipped.\n";
                        }
                    }
                    std::cout << "[Success] Added " << count << " children to " << h_name << "\n";
                } else {
                    std::cout << "[Error] House '" << h_name << "' not found!\n";
                }
            } else {
                std::cout << "[Usage] add_kids <house_name> <child1> <child2> ...\n";
            }
        }
        else if (cmd == "ask") {
            std::string nameA, nameB;
            if (ss >> nameA >> nameB) {
                if (people.count(nameA) && people.count(nameB)) {
                    std::string relation = get_english_kinship(people[nameA], people[nameB]);
                    std::cout << nameA << " calls " << nameB << " as: **" << relation << "**\n";
                } else {
                    std::cout << "[Error] One or both persons not found!\n";
                }
            } else {
                std::cout << "[Usage] ask <personA> <personB>\n";
            }
        }
        else {
            std::cout << "[Error] Unknown command.\n";
        }
    }

    // Clean memory
    for (auto& pair : people) delete pair.second;
    for (auto& pair : houses) delete pair.second;

    return 0;
}
