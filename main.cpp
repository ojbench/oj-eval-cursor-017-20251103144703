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

struct Train {
    string trainID;
    int stationNum = 0;
    int seatNum = 0;
    vector<string> stations;           // size = stationNum
    vector<int> prices;                 // size = stationNum - 1
    int startTimeMinutes = 0;           // minutes from 00:00
    vector<int> travelTimes;            // size = stationNum - 1 (minutes)
    vector<int> stopoverTimes;          // size = stationNum - 2 (minutes); empty if stationNum==2
    int saleStartDay = 0;               // days since 06-01
    int saleEndDay = 0;                 // days since 06-01
    char type = 'G';
    bool released = false;
};

static inline int parse_time_hhmm(const string &s) {
    // format HH:MM
    int hh = (s[0] - '0') * 10 + (s[1] - '0');
    int mm = (s[3] - '0') * 10 + (s[4] - '0');
    return hh * 60 + mm;
}

static inline int month_day_to_offset(int month, int day) {
    // 2021-06-01 is offset 0; months 6,7,8; ignoring leap years
    static int prefixDays[13] = {0,0,0,0,0,0, 0, 30, 61, 0,0,0,0};
    // prefixDays[6]=0; [7]=30 (June has 30); [8]=61 (June+July=61)
    return prefixDays[month] + (day - 1);
}

static inline int parse_mmdd_to_offset(const string &s) {
    // format MM-DD
    int mm = (s[0] - '0') * 10 + (s[1] - '0');
    int dd = (s[3] - '0') * 10 + (s[4] - '0');
    return month_day_to_offset(mm, dd);
}

static inline void offset_to_mmdd_hhmm(int dayOffset, int minutesInDay, string &mmdd, string &hhmm) {
    // map back to MM-DD and HH:MM within June-August
    // June (30 days) offsets [0,29], July (31 days) offsets [30,60], August (31 days) offsets [61,91]
    int mm = 6;
    int dd = 1;
    if (dayOffset < 30) {
        mm = 6; dd = 1 + dayOffset;
    } else if (dayOffset < 61) {
        mm = 7; dd = 1 + (dayOffset - 30);
    } else {
        mm = 8; dd = 1 + (dayOffset - 61);
    }
    char buf1[6];
    snprintf(buf1, sizeof(buf1), "%02d-%02d", mm, dd);
    mmdd.assign(buf1);
    int hh = minutesInDay / 60;
    int mi = minutesInDay % 60;
    char buf2[6];
    snprintf(buf2, sizeof(buf2), "%02d:%02d", hh, mi);
    hhmm.assign(buf2);
}

static inline vector<string> split_by_bar(const string &s) {
    vector<string> res;
    string cur;
    for (char c : s) {
        if (c == '|') { res.push_back(cur); cur.clear(); }
        else cur.push_back(c);
    }
    res.push_back(cur);
    return res;
}

static inline vector<int> split_ints_by_bar(const string &s) {
    vector<int> res;
    string cur;
    for (char c : s) {
        if (c == '|') {
            if (!cur.empty()) res.push_back(stoi(cur)); else res.push_back(0);
            cur.clear();
        } else cur.push_back(c);
    }
    if (!cur.empty()) res.push_back(stoi(cur)); else res.push_back(0);
    return res;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    unordered_map<string, User> users;
    unordered_map<string, Train> trains;

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
        // Skip leading bracketed index like "[1]"
        if (startIdx < tokens.size() && tokens[startIdx].size() >= 2 && tokens[startIdx].front() == '[' && tokens[startIdx].back() == ']') {
            ++startIdx;
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
        } else if (cmd == "add_train") {
            // -i -n -m -s -p -x -t -o -d -y
            string i = get("i");
            string n = get("n");
            string m = get("m");
            string s = get("s");
            string p = get("p");
            string x = get("x");
            string t = get("t");
            string o = get("o");
            string d = get("d");
            string y = get("y");
            if (i.empty() || n.empty() || m.empty() || s.empty() || p.empty() || x.empty() || t.empty() || d.empty() || y.empty()) {
                cout << -1 << '\n';
                continue;
            }
            if (trains.count(i)) { cout << -1 << '\n'; continue; }
            Train tr; tr.trainID = i; tr.stationNum = stoi(n); tr.seatNum = stoi(m); tr.type = y[0];
            tr.stations = split_by_bar(s);
            tr.prices = split_ints_by_bar(p);
            tr.startTimeMinutes = parse_time_hhmm(x);
            tr.travelTimes = split_ints_by_bar(t);
            if (tr.stationNum == 2) {
                tr.stopoverTimes.clear();
            } else {
                if (o != "_") tr.stopoverTimes = split_ints_by_bar(o);
            }
            auto dparts = split_by_bar(d);
            if (dparts.size() != 2) { cout << -1 << '\n'; continue; }
            tr.saleStartDay = parse_mmdd_to_offset(dparts[0]);
            tr.saleEndDay = parse_mmdd_to_offset(dparts[1]);
            // basic validity checks
            if ((int)tr.stations.size() != tr.stationNum) { cout << -1 << '\n'; continue; }
            if ((int)tr.prices.size() != tr.stationNum - 1) { cout << -1 << '\n'; continue; }
            if ((int)tr.travelTimes.size() != tr.stationNum - 1) { cout << -1 << '\n'; continue; }
            if (tr.stationNum > 2 && (int)tr.stopoverTimes.size() != tr.stationNum - 2) { cout << -1 << '\n'; continue; }
            trains[i] = std::move(tr);
            cout << 0 << '\n';
        } else if (cmd == "release_train") {
            string i = get("i");
            auto it = trains.find(i);
            if (it == trains.end() || it->second.released) { cout << -1 << '\n'; continue; }
            it->second.released = true;
            cout << 0 << '\n';
        } else if (cmd == "query_train") {
            string i = get("i");
            string d = get("d");
            auto it = trains.find(i);
            if (it == trains.end()) { cout << -1 << '\n'; continue; }
            int day = parse_mmdd_to_offset(d);
            if (day < it->second.saleStartDay || day > it->second.saleEndDay) { cout << -1 << '\n'; continue; }
            const Train &tr = it->second;
            cout << tr.trainID << ' ' << tr.type << '\n';
            // Precompute cumulative price and time
            vector<int> cumPrice(tr.stationNum, 0);
            for (int k = 1; k < tr.stationNum; ++k) cumPrice[k] = cumPrice[k - 1] + tr.prices[k - 1];
            // Compute absolute times
            int curDay = day;
            int curMinute = tr.startTimeMinutes; // leaving at station 1

            // Station 1
            {
                string arrMMDD = "xx-xx", arrHHMM = "xx:xx";
                string depMMDD, depHHMM;
                offset_to_mmdd_hhmm(curDay, curMinute, depMMDD, depHHMM);
                cout << tr.stations[0] << ' ' << arrMMDD << ' ' << arrHHMM << " -> "
                     << depMMDD << ' ' << depHHMM << ' ' << 0 << ' ' << tr.seatNum << '\n';
            }
            // Following stations
            int runningDay = curDay;
            int runningMinute = curMinute;
            for (int idx = 1; idx < tr.stationNum; ++idx) {
                // travel from idx-1 to idx
                runningMinute += tr.travelTimes[idx - 1];
                while (runningMinute >= 24 * 60) { runningMinute -= 24 * 60; runningDay += 1; }
                string arrMMDD, arrHHMM;
                offset_to_mmdd_hhmm(runningDay, runningMinute, arrMMDD, arrHHMM);

                string depMMDD = "xx-xx", depHHMM = "xx:xx";
                if (idx != tr.stationNum - 1) {
                    // stopover at station idx
                    int leaveDay = runningDay;
                    int leaveMinute = runningMinute + tr.stopoverTimes[idx - 1];
                    while (leaveMinute >= 24 * 60) { leaveMinute -= 24 * 60; leaveDay += 1; }
                    offset_to_mmdd_hhmm(leaveDay, leaveMinute, depMMDD, depHHMM);
                    // Update running position for next edge
                    runningDay = leaveDay;
                    runningMinute = leaveMinute;
                }

                // For last station, seat should be x
                if (idx == tr.stationNum - 1) {
                    cout << tr.stations[idx] << ' ' << arrMMDD << ' ' << arrHHMM << " -> "
                         << depMMDD << ' ' << depHHMM << ' ' << cumPrice[idx] << ' ' << 'x' << '\n';
                } else {
                    cout << tr.stations[idx] << ' ' << arrMMDD << ' ' << arrHHMM << " -> "
                         << depMMDD << ' ' << depHHMM << ' ' << cumPrice[idx] << ' ' << tr.seatNum << '\n';
                }
            }
        } else if (cmd == "clean") {
            users.clear();
            trains.clear();
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
