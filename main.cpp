#include <bits/stdc++.h>
using namespace std;

struct User {
    string username;
    string password;
    string name;
    string mail;
    int privilege = 0;
    bool loggedIn = false;
};

static inline bool starts_with_dash(const string &s) { return !s.empty() && s[0] == '-'; }

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    unordered_map<string, User> users;

    string line;
    while (true) {
        if (!getline(cin, line)) break;
        if (line.empty()) continue;

        // parse tokens by spaces
        vector<string> tokens;
        {
            string cur;
            for (size_t i = 0; i < line.size(); ++i) {
                char c = line[i];
                if (c == ' ') {
                    if (!cur.empty()) { tokens.push_back(cur); cur.clear(); }
                } else {
                    cur.push_back(c);
                }
            }
            if (!cur.empty()) tokens.push_back(cur);
        }
        if (tokens.empty()) continue;

        // Handle optional leading prompt token '>'
        size_t startIdx = 0;
        if (tokens.size() >= 2 && tokens[0] == ">") {
            startIdx = 1;
        }

        if (startIdx >= tokens.size()) continue;

        string cmd = tokens[startIdx];
        // build param map: key -> value
        unordered_map<string, string> param;
        for (size_t i = startIdx + 1; i < tokens.size(); ++i) {
            if (tokens[i].size() >= 2 && tokens[i][0] == '-' && tokens[i][1] != '\0') {
                string key(1, tokens[i][1]);
                string value;
                if (i + 1 < tokens.size() && !starts_with_dash(tokens[i + 1])) {
                    value = tokens[i + 1];
                    ++i;
                } else {
                    value = ""; // missing value, though inputs claim legal
                }
                param[key] = value;
            }
        }

        auto get = [&](const string &k) -> string {
            auto it = param.find(k);
            return it == param.end() ? string("") : it->second;
        };

        if (cmd == "add_user") {
            string u = get("u");
            string p = get("p");
            string n = get("n");
            string m = get("m");
            string gstr = get("g");
            string c = get("c");

            if (users.empty()) {
                if (u.empty() || p.empty() || n.empty() || m.empty()) {
                    cout << -1 << '\n';
                    continue;
                }
                if (users.count(u)) {
                    cout << -1 << '\n';
                    continue;
                }
                User nu; nu.username = u; nu.password = p; nu.name = n; nu.mail = m; nu.privilege = 10; nu.loggedIn = false;
                users[u] = nu;
                cout << 0 << '\n';
            } else {
                // need current user c logged in; new user's privilege < c's privilege
                if (u.empty() || p.empty() || n.empty() || m.empty() || gstr.empty() || c.empty()) {
                    cout << -1 << '\n';
                    continue;
                }
                auto itc = users.find(c);
                if (itc == users.end() || !itc->second.loggedIn) {
                    cout << -1 << '\n';
                    continue;
                }
                int g;
                try { g = stoi(gstr); } catch (...) { cout << -1 << '\n'; continue; }
                if (g >= itc->second.privilege) { cout << -1 << '\n'; continue; }
                if (users.count(u)) { cout << -1 << '\n'; continue; }
                User nu; nu.username = u; nu.password = p; nu.name = n; nu.mail = m; nu.privilege = g; nu.loggedIn = false;
                users[u] = nu;
                cout << 0 << '\n';
            }
        } else if (cmd == "login") {
            string u = get("u");
            string p = get("p");
            auto it = users.find(u);
            if (it == users.end()) { cout << -1 << '\n'; continue; }
            if (it->second.password != p) { cout << -1 << '\n'; continue; }
            if (it->second.loggedIn) { cout << -1 << '\n'; continue; }
            it->second.loggedIn = true;
            cout << 0 << '\n';
        } else if (cmd == "logout") {
            string u = get("u");
            auto it = users.find(u);
            if (it == users.end() || !it->second.loggedIn) { cout << -1 << '\n'; continue; }
            it->second.loggedIn = false;
            cout << 0 << '\n';
        } else if (cmd == "query_profile") {
            string c = get("c");
            string u = get("u");
            auto itc = users.find(c);
            auto itu = users.find(u);
            if (itc == users.end() || !itc->second.loggedIn || itu == users.end()) { cout << -1 << '\n'; continue; }
            bool ok = (itc->second.privilege > itu->second.privilege) || (c == u);
            if (!ok) { cout << -1 << '\n'; continue; }
            cout << itu->second.username << ' ' << itu->second.name << ' ' << itu->second.mail << ' ' << itu->second.privilege << '\n';
        } else if (cmd == "modify_profile") {
            string c = get("c");
            string u = get("u");
            auto itc = users.find(c);
            auto itu = users.find(u);
            if (itc == users.end() || !itc->second.loggedIn || itu == users.end()) { cout << -1 << '\n'; continue; }
            bool canView = (itc->second.privilege > itu->second.privilege) || (c == u);
            if (!canView) { cout << -1 << '\n'; continue; }
            // If -g present, must be lower than c's privilege
            string newp = get("p");
            string newn = get("n");
            string newm = get("m");
            string gstr = get("g");
            if (!gstr.empty()) {
                int g;
                try { g = stoi(gstr); } catch (...) { cout << -1 << '\n'; continue; }
                if (g >= itc->second.privilege) { cout << -1 << '\n'; continue; }
                itu->second.privilege = g;
            }
            if (!newp.empty()) itu->second.password = newp;
            if (!newn.empty()) itu->second.name = newn;
            if (!newm.empty()) itu->second.mail = newm;
            cout << itu->second.username << ' ' << itu->second.name << ' ' << itu->second.mail << ' ' << itu->second.privilege << '\n';
        } else if (cmd == "clean") {
            users.clear();
            cout << 0 << '\n';
        } else if (cmd == "exit") {
            cout << "bye\n";
            break;
        } else {
            // Unknown command: ignore line silently to be robust to prompts or comments
            // (This avoids spurious outputs if inputs contain non-command lines.)
            continue;
        }
    }
    return 0;
}
