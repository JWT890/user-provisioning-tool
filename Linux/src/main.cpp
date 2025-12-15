#define _POSIX_C_SOURCE 200809L
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <unistd.h>


using namespace std;

string generatePassword() {
    const string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$";
    string password;
    srand(static_cast<unsigned int>(time(0)));

    for (int i = 0; i < 12; i++) {
        password += chars[static_cast<size_t>(rand()) % chars.length()];
    }
    return password;
}

bool isRoot() {
    return getpid() == 0;
}

void logAction(const string& action, const string& username, bool success) {
    time_t now = time(nullptr);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    cout << "[" << timestamp << "] " << action << "| User: " << username
         << " | " << (success ? "SUCCESS" : "FAILURE") << endl;
}

void createUser(const string& username, const string& fulltime = "", const string& groups = "") {
    if (!isRoot()) {
        cerr << "Error: Must be run as sudo" << endl;
    }

    string checkCmd = "id" + username + "> /dev.null 2>&1";
    if (system(checkCmd.c_str()) == 0) {
        cerr << "Error: User already exists" << endl;
        logAction("CREATE", username, false);
        return;
    }

    string cmd = "useradd -m";

    if (!fulltime.empty()) {
        cmd += " -c \"" + fulltime + "\"";
    }

    if (!groups.empty()) {
        cmd += " -G " + groups;
    }

    cmd += " " + username;

    if (system(cmd.c_str()) !=0) {
        cerr << "Error: Falied to create user" << endl;
        logAction("CREATE", username, false);
        return;
    }

    string password = generatePassword();
    string passwdCmd = "echo '" + username + ":" + password + "' | chpasswd";
    system(passwdCmd.c_str());

    system(("chage -d 0" + username).c_str());

    logAction("CREATE", username, true);

    cout << "\nUser created successfully!" << endl;
    cout << "Username: " << username << endl;
    cout << "Temperorary password: " << password << endl;
    cout << "User must change password at first login.\n" << endl;
}

void deleteUser(const string& username, bool removeHome) {
    if (!isRoot()) {
        cerr << "Error: Must run with sudo" << endl;
        return;
    }

    string cmd = "userdel"; 
    if (removeHome) {
        cmd += " -r";
    }
    cmd += " " + username;

    if (system(cmd.c_str()) == 0) {
        logAction("DELETE", username, true);
        cout << "User deleted successfully!" << endl;
    } else {
        logAction("DELETE", username, false);
        cerr << "Error: Falied to delete user" << endl;
    }
}

void disableUser(const string& username) {
    if (!isRoot()) {
        cerr << "Error: Must run with sudo" << endl;
        return;
    }

    string cmd = "usermod -L" + username;

    if (system(cmd.c_str()) == 0) {
        logAction("DISABLE", username, true);
        cout << "User disabled successfully!" << endl;
    } else {
        logAction("DISABLE", username, false);
        cerr << "Error: Falied to disable user" << endl;
    }
}

void enableUser(const string& username) {
    if (!isRoot()) {
        cerr << "Error: Must run with sudo" << endl;
        return;
    }

    string cmd = "usermod -U" + username;

    if (system(cmd.c_str()) == 0) {
        logAction("ENABLE", username, true);
        cout << "User enabled successfully!" << endl;
    } else {
        logAction("ENABLE", username, false);
        cerr << "Error: Falied to enable user" << endl;
    }
}

void listUsers() {
    cout << "\n=== System Users ===" << endl;
    system("cut -d: -f1,3 /etc/passwd | awk -F: '$2 >= 1000 {print $1}'");
    cout << endl;
}

void printHelp() {
    cout << "User Provisioning Tool" << endl;
    cout << "Usage: ./user-provisioning-tool [command] [options]" << endl;
    cout << "Commands:" << endl;
    cout << "  create <username> [--full-name <name>] [--groups <group1,group2>]  Create a new user" << endl;
    cout << "  delete <username> [--remove-home]                                   Delete a user" << endl;
    cout << "  disable <username>                                                 Disable a user account" << endl;
    cout << "  enable <username>                                                  Enable a user account" << endl;
    cout << "  list                                                              List all users" << endl;
    cout << "  help                                                              Show this help message" << endl;
    cout << "\nExamples:" << endl;
    cout << " sudo ./userprov create john_doe --full-name \"John Doe\" --groups sudo,developers" << endl;
    cout << " sudo ./userprov delete john_doe --remove-home" << endl;
    cout << " sudo ./userprov disable john_doe" << endl;
    cout << " sudo ./userprov enable john_doe" << endl;
    cout << " ./userprov list" << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printHelp();
        return 1;
    }

    string command = argv[1];

    if (command == "create") {
        if (argc < 3) {
            cerr << "Error: Username required" << endl;
            return 1;
        }
        string username = argv[2];
        string fullname = (argc > 3) ? argv[3] : "";
        string groups = (argc > 4) ? argv[4] : "";
        createUser(username, fullname, groups);
    }
    else if (command == "delete") {
        if (argc < 3) {
            cerr << "Error: Username required" << endl;
            return 1;
        }
        string username = argv[2];
        bool removeHome = (argc > 3 && string(argv[3]) == "--remove-home");
        deleteUser(username, removeHome);
    }
    else if (command == "disable") {
        if (argc < 3) {
            cerr << "Error: Username required" << endl;
            return 1;
        }
        disableUser(argv[2]);
    }
    else if (command == "enable") {
        if (argc < 3) {
            cerr << "Error: Username required" << endl;
            return 1;
        }
        enableUser(argv[2]);
    }
    else if (command == "list") {
        listUsers();
    }
    else {
        cerr << "Unknown command: " << command << endl;
        printHelp();
        return 1;
    }

    return 0;
}