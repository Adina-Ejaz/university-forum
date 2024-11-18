#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <utility>
#include <limits>
#include <conio.h>

using namespace std;

// Global variables
string Pusername;
string Prole;

// Base class: User
class User {
public:
    User(const string& username, const string& password, const string& role)
        : username(username), password(password), role(role) {}
    // static function so it can be called in main without making  any object of this class
    static bool AuthenticateUser() {
    	//vector of type user used to store users
        vector<User> users = LoadUsersFromFile("users.txt");

        string enteredUsername, enteredPassword;
        cout << "Enter username: ";
        cin >> enteredUsername;
        cout << "Enter password: ";
        cin >> enteredPassword;

        return CheckCredentials(users, enteredUsername, enteredPassword);
    }

protected:
    string username;
    string password;
    string role;

    static vector<User> LoadUsersFromFile(const string& filename) {
        vector<User> users;
        ifstream file(filename);
        if (!file) {
            cerr << "Error opening file: " << filename << endl;
            return users;
        }

        string line;
        while (getline(file, line)) {
            string username = line;
            if (!getline(file, line)) break;
            string password = line;
            if (!getline(file, line)) break;
            string role = line;
            users.emplace_back(username, password, role);
        }

        file.close();
        return users;
    }

    static bool CheckCredentials(const vector<User>& users, const string& enteredUsername, const string& enteredPassword) {
        for (const auto& user : users) {
            if (user.username == enteredUsername && user.password == enteredPassword) {
                cout << "Login successful! Role: " << user.role << endl;
                Prole = user.role;
                Pusername = user.username;
                cout << "Press any key to continue...";
                _getch();
                system("cls");
                return true;
            }
        }
        cout << "Invalid username or password." << endl;
        cout << "Press any key to continue...";
        _getch();
        system("cls");
        return false;
    }
};

// Base class: ForumElement
class ForumElement {
public:
	//pure virtual functions
    virtual void Post(const string& title, const string& content) = 0;
    virtual void Display(const string& filename) = 0;
};

// Derived class: Message
class Message : public ForumElement {
public:
    void Post(const string& threadTitle, const string& message) override {
        ofstream file(threadTitle, ios::app);
        if (!file.is_open()) {
            cerr << "Error: Unable to open thread file \"" << threadTitle << "\"" << endl;
            return;
        }

        time_t now = time(nullptr);
        char timestamp[100];
        if (strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now)) == 0) {
            cerr << "Error formatting timestamp" << endl;
            return;
        }

        file << "Author ID: " << Pusername<< endl;
        file << "Timestamp: " << timestamp << endl;
        file << "Message: " << message << endl;
        file << "=====================" << endl;
        file.close();

        cout << "Message posted successfully on thread \"" << threadTitle << "\"" << endl;
    }

    void Display(const string& filename) override {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Unable to open thread file \"" << filename << "\"" << endl;
            return;
        }

        string line;
        while (getline(file, line)) {
            cout << line << endl;
        }
        file.close();
    }
};

// Derived class: Thread
class Thread : public ForumElement {
public:
    void Post(const string& title, const string& message) override {
        ifstream count("count.txt");
        string line;
        int countline = 0;
        while (getline(count, line)) {
            countline++;
        }
        lastThreadNumber = countline;

        ofstream inputCount("count.txt", ios::app);
        string fileName = "thread" + to_string(++lastThreadNumber) + ".txt";
        ofstream file(fileName);
        
        if (file.is_open()) {
            file << "Title: " << title << endl;
            file << "Author: " << Pusername << endl;
            file << "Timestamp: " << GetTimestamp() << endl;
            file << "=====================" << endl;
            file.close();
            
            cout << "Thread \"" << title << "\" created successfully." << endl;
            inputCount << "yes" << endl;

            int postMessage;
            cout << "Enter 1 if you would like to post a message: ";
            cin >> postMessage;
            if (postMessage == 1) {
                string message;
                cout << "Enter the message you would like to post: ";
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                getline(cin, message);
                Message msg;
                msg.Post(fileName, message);
            } else {
                cout << "No message will be posted." << endl;
            }
        } else {
            cerr << "Error creating thread \"" << title << "\"." << endl;
        }
    }

    void Display(const string& threadTitle) override {
        ifstream file(threadTitle);
        if (!file.is_open()) {
            cerr << "Error: Unable to open thread file \"" << threadTitle << "\"" << endl;
            return;
        }

        string line;
        while (getline(file, line)) {
            cout << line << endl;
        }
        file.close();
    }

    static pair<vector<string>, vector<string>> SearchThreadsByKeyword(const string& keyword) {
        vector<string> matchingThreads;
        vector<string> fileNamesOfMT;
        for (int i = 2;; ++i) {
            string filename = "thread" + to_string(i) + ".txt";
            ifstream file(filename);
            if (file.is_open()) {
                string title;
                if (getline(file, title)) {
                    if (title.find(keyword) != string::npos) {
                        matchingThreads.push_back(title);
                        fileNamesOfMT.push_back(filename);
                    }
                }
                file.close();
            } else {
                break;
            }
        }

        return make_pair(matchingThreads, fileNamesOfMT);
    }

    static void EraseFileContents(const string& filename) {
        if (Prole == "teacher") {
            ofstream file(filename, ios::trunc);
            if (!file.is_open()) {
                cerr << "Error: Could not open the file " << filename << endl;
                return;
            }
            cout << "File " << filename << " has been cleared." << endl;
            file.close();
        } else {
            cout << "You do not have permission to delete threads." << endl;
        }
    }

private:
    static int lastThreadNumber;

    static string GetTimestamp() {
        time_t now = time(nullptr);
        char buf[100];
        if (strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now))) {
            return buf;
        }
        return "";
    }
};

int Thread::lastThreadNumber = 0;

class Forum {
public:
    void Run() {
        if (!User::AuthenticateUser()) {
            return;
        }

        int choice;
        while (true) {
            cout << "Enter 1 to Create Thread" << endl;
            cout << "Enter 2 to Search For Thread" << endl;
            cout << "Enter 3 to Delete A Thread" << endl;
            cout << "Enter 4 to Exit" << endl;
            cin >> choice;

            switch (choice) {
                case 1:
                    CreateThread();
                    break;
                case 2:
                    SearchThread();
                    break;
                case 3:
                    DeleteThread();
                    break;
                case 4:
                    return;
                default:
                    cout << "Invalid option" << endl;
            }
        }
    }

private:
    void CreateThread() {
        string title, id;
        cout << "Enter title of thread: ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, title);
        
        ForumElement* ptr;
        Thread thread;
        ptr=&thread;
        ptr->Post(title, "");
    }

    void SearchThread() {
        string keyword;
        cout << "Enter keyword to search threads: ";
        cin >> keyword;

        auto result = Thread::SearchThreadsByKeyword(keyword);
        auto& matchingThreads = result.first;
        auto& fileNamesOfMT = result.second;

        if (matchingThreads.empty()) {
            cout << "No matching threads found." << endl;
        } else {
            cout << "Matching threads found:" << endl;
            for (int i = 0; i < matchingThreads.size(); ++i) {
                cout << i + 1 << ". " << matchingThreads[i] << endl;
            }

            int choice;
            cout << "Enter the index of the thread to open: ";
            cin >> choice;

            while (cin.fail() || choice < 1 || choice > matchingThreads.size()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid choice. Please enter a valid index: ";
                cin >> choice;
            }

            cout << "Press any key to continue...";
            _getch();
            system("cls");
            ForumElement* baseptr;
            
            Thread thread;
            baseptr=&thread;
            baseptr->Display(fileNamesOfMT[choice - 1]);

            int postMessage;
            cout << "Enter 1 if you would like to post a message: ";
            cin >> postMessage;
            if (postMessage == 1) {
                string message;
                cout << "Enter the message you would like to post: ";
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                getline(cin, message);
                ForumElement* messageptr;
                
                Message msg;
                messageptr=&msg;
                messageptr->Post(fileNamesOfMT[choice - 1], message);
                
            } else {
                cout << "No message will be posted." << endl;
            }
        }
    }

    void DeleteThread() {
        string filename;
        cout << "Enter filename of thread you wish to erase contents for: ";
        cin >> filename;
        Thread::EraseFileContents(filename);
    }
};

int main() {
    Forum forum;
    forum.Run();
    return 0;
}

