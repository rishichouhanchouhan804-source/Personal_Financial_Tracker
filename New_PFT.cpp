#include <bits/stdc++.h>
using namespace std;

// Utility functions
static inline string trim(const string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Validate DD-MM-YYYY format
bool valid_date_format(const string &date) {
    if (date.size() != 10) return false;
    // Check digits and dashes
    if (!isdigit(date[0]) || !isdigit(date[1])) return false; // DD
    if (date[2] != '-') return false;
    if (!isdigit(date[3]) || !isdigit(date[4])) return false; // MM
    if (date[5] != '-') return false;
    if (!isdigit(date[6]) || !isdigit(date[7]) || !isdigit(date[8]) || !isdigit(date[9])) return false; // YYYY
    return true;
}

// Extract "MM-YYYY" for monthly grouping
string month_key(const string &date) {
    if (!valid_date_format(date)) return "00-0000";
    // date = DD-MM-YYYY → extract MM-YYYY
    return date.substr(3, 7);
}

// Class Declarations
class Account;

class Transaction {
protected:
    double amount;
    string category;
    string date;
    string description;

public:
    Transaction(double amt, string cat, string dt, string desc = "")
        : amount(amt), category(move(cat)), date(move(dt)), description(move(desc)) {}

    virtual ~Transaction() = default;

    virtual void apply(Account &acc) const = 0;
    virtual string type() const = 0;

    double getAmount() const { return amount; }
    string getCategory() const { return category; }
    string getDate() const { return date; }
    string getDesc() const { return description; }
};

// Account Class
class Account {
private:
    double balance;
    vector<shared_ptr<Transaction>> transactions;

public:
    Account() : balance(0.0) {}

    void deposit(double amt) { balance += amt; }
    void withdraw(double amt) { balance -= amt; }

    double getBalance() const { return balance; }

    bool addTransaction(shared_ptr<Transaction> tx) {
        if (!tx) return false;
        if (tx->getAmount() <= 0.0) {
            cout << "Error: amount must be positive.\n";
            return false;
        }
        if (!valid_date_format(tx->getDate())) {
            cout << "Error: invalid date format.\n";
            return false;
        }
        tx->apply(*this);
        transactions.push_back(move(tx));
        return true;
    }

    const vector<shared_ptr<Transaction>>& getTransactions() const { return transactions; }

    vector<shared_ptr<Transaction>> getTransactionsForMonth(const string &monthYear) const {
        vector<shared_ptr<Transaction>> out;
        for (auto &t : transactions)
            if (month_key(t->getDate()) == monthYear)
                out.push_back(t);
        return out;
    }
};

// Derived Classes
class Income : public Transaction {
public:
    Income(double amt, string cat, string dt, string desc = "")
        : Transaction(amt, move(cat), move(dt), move(desc)) {}
    void apply(Account &acc) const override { acc.deposit(amount); }
    string type() const override { return "Income"; }
};

class Expense : public Transaction {
public:
    Expense(double amt, string cat, string dt, string desc = "")
        : Transaction(amt, move(cat), move(dt), move(desc)) {}
    void apply(Account &acc) const override { acc.withdraw(amount); }
    string type() const override { return "Expense"; }
};

// ReportGenerator
class ReportGenerator {
public:
    struct Summary {
        double totalIncome = 0.0;
        double totalExpense = 0.0;
        double net = 0.0;
        unordered_map<string, double> byCategoryIncome;
        unordered_map<string, double> byCategoryExpense;
    };

    Summary generateMonthlyReport(const Account &acc, const string &monthYear) const {
        Summary s;
        auto txs = acc.getTransactionsForMonth(monthYear);
        for (auto &t : txs) {
            if (t->type() == "Income") {
                s.totalIncome += t->getAmount();
                s.byCategoryIncome[t->getCategory()] += t->getAmount();
            } else {
                s.totalExpense += t->getAmount();
                s.byCategoryExpense[t->getCategory()] += t->getAmount();
            }
        }
        s.net = s.totalIncome - s.totalExpense;
        return s;
    }

    void printMonthlyReport(const Account &acc, const string &monthYear) const {
        auto s = generateMonthlyReport(acc, monthYear);
        cout << "\n===== Monthly Report (" << monthYear << ") =====\n";
        cout << "Total Income : " << s.totalIncome << "\n";
        cout << "Total Expense: " << s.totalExpense << "\n";
        cout << "Net Savings  : " << s.net << "\n";
        cout << "\n-- Income by Category --\n";
        if (s.byCategoryIncome.empty()) cout << "None\n";
        else for (auto &p : s.byCategoryIncome) cout << "  " << p.first << ": " << p.second << "\n";
        cout << "\n-- Expense by Category --\n";
        if (s.byCategoryExpense.empty()) cout << "None\n";
        else for (auto &p : s.byCategoryExpense) cout << "  " << p.first << ": " << p.second << "\n";
        cout << "============================\n";
        cout << "Current Balance: Rs." << acc.getBalance() << "\n";
    }

    void printAllTimeReport(const Account &acc) const {
        double income = 0, expense = 0;
        for (auto &t : acc.getTransactions()) {
            if (t->type() == "Income") income += t->getAmount();
            else expense += t->getAmount();
        }
        cout << "\n===== All-Time Summary =====\n";
        cout << "Total Income : " << income << "\n";
        cout << "Total Expense: " << expense << "\n";
        cout << "Net Savings  : " << (income - expense) << "\n";
        cout << "============================\n";
    }
};

// CLI
void showMenu(double balance) {
    cout << "\n--------------------------------\n";
    cout << "   Personal Finance Tracker     \n";
    cout << "--------------------------------\n";
    cout << fixed << setprecision(2);
    cout << "Current Balance: Rs." << balance << "\n";
    cout << "--------------------------------\n";
    cout << "1. Add Income\n";
    cout << "2. Add Expense\n";
    cout << "3. View Monthly Report\n";
    cout << "4. View All-time Summary\n";
    cout << "5. Exit\n";
    cout << "Choose an option: " << flush;
}

double readAmount() {
    while (true) {
        string input;
        getline(cin, input);
        try {
            double amt = stod(trim(input));
            if (amt > 0) return amt;
        } catch (...) {}
        cout << "Enter a valid positive amount: " << flush;
    }
}

int main() {
    Account acc;
    ReportGenerator rg;

    while (true) {
        showMenu(acc.getBalance());
        string choice;
        getline(cin, choice);
        choice = trim(choice);

        if (choice == "1" || choice == "2") {
            bool isIncome = (choice == "1");
            cout << (isIncome ? "Adding Income...\n" : "Adding Expense...\n");
            cout << "Amount: " << flush;
            double amt = readAmount();
            cout << "Category: " << flush;
            string cat; getline(cin, cat); cat = trim(cat);
            cout << "Date (DD-MM-YYYY): " << flush;
            string dt; getline(cin, dt); dt = trim(dt);
            if (!valid_date_format(dt)) {
                cout << "Invalid date format! Use DD-MM-YYYY.\n";
                continue;
            }
            cout << "Description (optional): " << flush;
            string desc; getline(cin, desc); desc = trim(desc);

            shared_ptr<Transaction> tx;
            if (isIncome)
                tx = make_shared<Income>(amt, cat, dt, desc);
            else
                tx = make_shared<Expense>(amt, cat, dt, desc);

            if (acc.addTransaction(tx)) {
                cout << "Transaction added successfully!\n";
                cout << " Current Balance: Rs." << fixed << setprecision(2) << acc.getBalance() << "\n";
            }
        }
        else if (choice == "3") {
            cout << "Enter month and year (MM-YYYY): " << flush;
            string my; getline(cin, my); my = trim(my);
            rg.printMonthlyReport(acc, my);
        }
        else if (choice == "4") {
            rg.printAllTimeReport(acc);
        }
        else if (choice == "5") {
            cout << "Goodbye!, Kushagra\n";
            break;
        }
        else {
            cout << "Invalid option. Try again.\n";
        }
    }

    return 0;
}
