#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <iostream>
#include <string>
#include <QMessageBox>
#include <QScreen>
#include <QStyle>
#include <QDateTime>
#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

#include <QStyleFactory>
#include <QListView>

#include <fstream>
#include <QInputDialog>
using namespace std;

// Constants
const int MAX_USERS = 10;
const int MAX_ACCOUNTS = 3;
const int MAX_TRANSACTIONS = 10;
const int PHONE_LENGTH = 11;
const int CVV_LENGTH = 3;
const int CARD_NUMBER = 16;
int currentUserIndex = -1;

// Page indices
const int WELCOME_PAGE = 0;
const int LOGIN_PAGE = 1;
const int SIGNUP_PAGE = 2;
const int DASHBOARD_PAGE = 3;
const int TRANSFER_PAGE = 4;
const int HISTORY_PAGE = 5;
const int DONATION_PAGE = 6;
const int ADD_ACCOUNT_PAGE = 7;
const int VIEW_BALANCE = 8;
const int DEPOSIT = 9;

// Structs
struct transaction {
    string from;
    string to;
    long double amount = -1;
};

struct account {
    string cardNo;
    string holderName;
    string cvvCode = "-";
    string expirationDate;
    string bankName;
    long double balance = 0;
    transaction trans[MAX_TRANSACTIONS];
    int transaction_count = 0;
};

struct user {
    string id;
    string username;
    string password;
    string email;
    string address;
    string phone;
    account accounts[MAX_ACCOUNTS];
    int num_of_accounts = 0;
};

// Global array
user users[MAX_USERS] = {};

void MainWindow::saveDataToFile() {
    ofstream outFile("database.txt");
    if (!outFile) return;

    for (int i = 0; i < MAX_USERS; i++) {
        if (!users[i].username.empty()) {
            outFile << users[i].username << "|"
                    << users[i].password << "|"
                    << users[i].phone << "|"
                    << users[i].email << "|"
                    << users[i].address << "|"
                    << users[i].num_of_accounts << endl;

            for (int j = 0; j < users[i].num_of_accounts; j++) {
                outFile << users[i].accounts[j].bankName << "|"
                        << users[i].accounts[j].cardNo << "|"
                        << users[i].accounts[j].balance << "|"
                        << users[i].accounts[j].transaction_count << endl;

                for (int k = 0; k < users[i].accounts[j].transaction_count; k++) {
                    outFile << users[i].accounts[j].trans[k].from << "#"
                            << users[i].accounts[j].trans[k].to << "#"
                            << users[i].accounts[j].trans[k].amount << endl;
                }
            }
        }
    }
    outFile.close();
}

void MainWindow::loadDataFromFile() {
    ifstream inFile("database.txt");
    if (!inFile) return;

    string temp;
    int i = 0;
    while (i < MAX_USERS && getline(inFile, users[i].username, '|')) {
        getline(inFile, users[i].password, '|');
        getline(inFile, users[i].phone, '|');
        getline(inFile, users[i].email, '|');
        getline(inFile, users[i].address, '|');
        getline(inFile, temp);

        if(!temp.empty()) users[i].num_of_accounts = stoi(temp);

        for (int j = 0; j < users[i].num_of_accounts; j++) {
            getline(inFile, users[i].accounts[j].bankName, '|');
            getline(inFile, users[i].accounts[j].cardNo, '|');

            getline(inFile, temp, '|');
            if(!temp.empty()) users[i].accounts[j].balance = stold(temp);

            getline(inFile, temp);
            if(!temp.empty()) users[i].accounts[j].transaction_count = stoi(temp);

            for (int k = 0; k < users[i].accounts[j].transaction_count; k++) {
                getline(inFile, users[i].accounts[j].trans[k].from, '#');
                getline(inFile, users[i].accounts[j].trans[k].to, '#');
                getline(inFile, temp);
                if(!temp.empty()) users[i].accounts[j].trans[k].amount = stold(temp);
            }
        }
        i++;
    }
    inFile.close();
}



// Functions
int findUserByPhone(string phone, user users[]) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (!users[i].phone.empty() && users[i].phone == phone) return i;
    }
    return -1;
}

bool isExistPhone(string phone, user users[]) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (!users[i].phone.empty() && users[i].phone == phone) return true;
    }
    return false;
}

bool isValidPhone(string phone, user users[]) {
    if (phone.length() != PHONE_LENGTH || phone[0] != '0' || phone[1] != '1' || (phone[2] > '2' && phone[2] < '5') || phone[2] > '5') return false;
    for (char c : phone) { if (c < '0' || c > '9') return false; }
    return !isExistPhone(phone, users);
}

bool isEmailTaken(string email, user users[]) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (!users[i].email.empty() && users[i].email == email) {
            return true;
        }
    }
    return false;
}

bool isUsernameTaken(string username, user users[]) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (!users[i].username.empty() && users[i].username == username) {
            return true;
        }
    }
return false;
}
bool isValidAccount(string cardNo, string cvv, string expDate) {
    if (cardNo.empty() || cvv.empty() || expDate.empty()) return false;
    if (cardNo.length() != CARD_NUMBER) return false;
    for (char c : cardNo) { if (!isdigit(c)) return false; }
    if (cvv.length() != CVV_LENGTH) return false;
    for (char c : cvv) { if (!isdigit(c)) return false; }
    if (expDate.length() != 5 || expDate[2] != '/') return false;
    string monthStr = expDate.substr(0, 2);
    if (!isdigit(monthStr[0]) || !isdigit(monthStr[1])) {
        return false;
    }

    int month = stoi(monthStr);
    if (month < 1 || month > 12) {
        return false;
    }

    string yearStr = expDate.substr(3, 2);
    if (isdigit(yearStr[0]) && isdigit(yearStr[1])) {
        int year = stoi(yearStr);
        if (year < 26 || year > 30) {
            return false;
        }
    } else {
        return false;
    }
    return true;
}


bool MainWindow::confirmWithPassword() {
    bool ok;
    QString password = QInputDialog::getText(this, "Security Confirmation",
                                             "Please enter your account password to confirm this action:",
                                             QLineEdit::Password, "", &ok);

    if (ok && !password.isEmpty()) {
        if (password.toStdString() == users[currentUserIndex].password) {
            return true;
        } else {
            QMessageBox::critical(this, "Error", "Incorrect password! Action cancelled.");
        }
    }
    return false;
}


int login(user usersArray[MAX_USERS], string inputUserName, string inputPassword) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (!usersArray[i].username.empty() &&
            usersArray[i].username == inputUserName &&
            usersArray[i].password == inputPassword) {
            return i;
        }
    }
    return -1;
}

void transferTo(account& fromAccount, account& toAccount, long double amount) {
    fromAccount.balance -= amount;
    toAccount.balance += amount;
}

void recordTransaction(account& acc, string from, string to, long double amount) {
    if (acc.transaction_count >= MAX_TRANSACTIONS) {
        for (int i = 0; i < MAX_TRANSACTIONS - 1; i++) { acc.trans[i] = acc.trans[i + 1]; }
        acc.transaction_count = MAX_TRANSACTIONS - 1;
    }
    int idx = acc.transaction_count;
    acc.trans[idx].from = from;
    acc.trans[idx].to = to;
    acc.trans[idx].amount = amount;
    acc.transaction_count++;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(qApp, &QCoreApplication::aboutToQuit, this, &MainWindow::saveDataToFile);
    loadDataFromFile();

    ui->bankNameEdit->setPlaceholderText("Bank Name (e.g., CIB, Banque Misr)");
    ui->cardNumberEdit->setPlaceholderText("16-Digit Card Number");
    ui->cvvEdit->setPlaceholderText("3-Digit CVV");
    ui->expiryDateEdit->setPlaceholderText("Expiry Date (MM/YY)");

    ui->UserEdit->setPlaceholderText("Username");
    ui->PassEdit->setPlaceholderText("Password");
    ui->transferPhoneEdit->setPlaceholderText("Receiver's phone number");
    ui->amountEdit->setPlaceholderText("Amount to transfer (e.g.)");
    ui->donationAmountEdit->setPlaceholderText("Enter your donation amount here...");
    ui->depositAmountEdit->setPlaceholderText(" amount you wish to deposit...");

    ui->User->setPlaceholderText("User_name");
    ui->Password->setPlaceholderText("Password");
    ui->EmailEdit->setPlaceholderText("Email");
    ui->AddressEdit->setPlaceholderText("Address");
    ui->Phone->setPlaceholderText("Phone_number");

    this->setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            this->size(),
            qApp->primaryScreen()->availableGeometry()
            )
        );

    QWidget *welcomePage = new QWidget(this);
    QVBoxLayout *welcomeLayout = new QVBoxLayout(welcomePage);

    QLabel *welcomeLabel = new QLabel("Welcome to InstaPay", welcomePage);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = welcomeLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    welcomeLabel->setFont(titleFont);
    welcomeLabel->setStyleSheet("color: #2c3e50; margin: 40px 0;");

    QPushButton *loginBtn = new QPushButton("Login", welcomePage);
    QPushButton *signupBtn = new QPushButton("Sign Up", welcomePage);
    QPushButton *exitBtn = new QPushButton("Exit", welcomePage);

    QString buttonStyle = "QPushButton { background-color: #3498db; color: white; border-radius: 5px; padding: 8px; font-size: 14px; }"
                          "QPushButton:hover { background-color: #2980b9; }";
    loginBtn->setStyleSheet(buttonStyle);
    signupBtn->setStyleSheet(buttonStyle);
    exitBtn->setStyleSheet(buttonStyle);

    welcomeLayout->addWidget(welcomeLabel);
    welcomeLayout->addWidget(loginBtn);
    welcomeLayout->addWidget(signupBtn);
    welcomeLayout->addWidget(exitBtn);
    welcomeLayout->setSpacing(20);
    welcomeLayout->setContentsMargins(50, 100, 50, 100);

    ui->stackedWidget->insertWidget(WELCOME_PAGE, welcomePage);
    ui->stackedWidget->setCurrentIndex(WELCOME_PAGE);

    connect(loginBtn, &QPushButton::clicked, this, &MainWindow::on_welcomeLoginBtn_clicked);
    connect(signupBtn, &QPushButton::clicked, this, &MainWindow::on_welcomeSignupBtn_clicked);
    connect(exitBtn, &QPushButton::clicked, this, &MainWindow::on_welcomeExitBtn_clicked);
}

MainWindow::~MainWindow() {
    delete ui;
    saveDataToFile();
}

void MainWindow::on_welcomeLoginBtn_clicked() {
    ui->stackedWidget->setCurrentIndex(LOGIN_PAGE);
}

void MainWindow::on_welcomeSignupBtn_clicked() {
    ui->stackedWidget->setCurrentIndex(SIGNUP_PAGE);
}

void MainWindow::on_welcomeExitBtn_clicked() {
    QApplication::quit();
}

void MainWindow::on_toLogInBtn_clicked() {



        ui->User->clear();
        ui->Password->clear();
        ui->EmailEdit->clear();
        ui->AddressEdit->clear();
        ui->Phone->clear();


    ui->stackedWidget->setCurrentIndex(LOGIN_PAGE);
}

void MainWindow::on_loginBtn_clicked() {

    ui->User->clear();
    ui->Password->clear();
    ui->EmailEdit->clear();
    ui->AddressEdit->clear();
    ui->Phone->clear();

    string inputUser = ui->UserEdit->text().toStdString();
    string inputPass = ui->PassEdit->text().toStdString();

    int userIndex = login(users, inputUser, inputPass);

    if (userIndex != -1) {
        currentUserIndex = userIndex;

        ui->stackedWidget->setCurrentIndex(DASHBOARD_PAGE);
    } else {
        QMessageBox::critical(this, "Error", "Invalid credentials!");
    }
}

void MainWindow::on_toSignUpBtn_clicked() {
    ui->stackedWidget->setCurrentIndex(SIGNUP_PAGE);
}

void MainWindow::on_SignUpBtn_clicked() {
    string sUser = ui->User->text().toStdString();
    string sPass = ui->Password->text().toStdString();
    string sPhone = ui->Phone->text().toStdString();
    string sEmail = ui->EmailEdit->text().toStdString();
    string sAddress = ui->AddressEdit->text().toStdString();
    if (sUser.empty()) {
        QMessageBox::warning(this, "Missing Info", "Username cannot be empty.");
        return;
    }

    if (isUsernameTaken(sUser, users)) {
        QMessageBox::warning(this, "Registration Error",
                             "This username is already taken. Please choose another one.");
        return;
    }

    for (char c : sUser) {
        if (!isalpha(c)) {
            QMessageBox::warning(this, "Invalid Username", "Numbers and symbols are not allowed in the username.");
            return;
        }
    }


    if (sUser.empty() || sPass.empty() || sPhone.empty() || sEmail.empty() || sAddress.empty())
    {
        QMessageBox::warning(this, "Missing Info", "Please fill in all fields (Username, Password, Phone, Email, and Address).");
        return;
    }

    if (isEmailTaken(sEmail, users)) {
        QMessageBox::warning(this, "Registration Error", "This email is already linked to another user.");
        return;
    }

    if (!isValidPhone(sPhone, users))
    {
        QMessageBox::warning(this, "Registration Error", "Invalid phone number or already registered!");
        return;
    }

    for (int i = 0; i < MAX_USERS; i++)
    {
        if (users[i].username.empty())
        {
            users[i].username = sUser;
            users[i].password = sPass;
            users[i].phone = sPhone;
            users[i].email = sEmail;
            users[i].address = sAddress;

            currentUserIndex = i;

            QMessageBox::information(this, "Success", "User created successfully!");
            ui->stackedWidget->setCurrentIndex(ADD_ACCOUNT_PAGE);
            return;
        }
    }
    QMessageBox::critical(this, "System Error", "User database is full.");
}

void MainWindow::on_TransferBtn_clicked() {
    if (currentUserIndex != -1) {
        ui->transferFromCombo->clear();
        for (int i = 0; i < users[currentUserIndex].num_of_accounts; i++) {
            string bankName = users[currentUserIndex].accounts[i].bankName;
            ui->transferFromCombo->addItem(QString::fromStdString(bankName));
        }
        if (users[currentUserIndex].num_of_accounts == 0) {
            QMessageBox::warning(this, "No Accounts", "Please add a bank account first!");
            ui->stackedWidget->setCurrentIndex(ADD_ACCOUNT_PAGE);
        } else {
            ui->stackedWidget->setCurrentIndex(TRANSFER_PAGE);
        }
    }
}

void MainWindow::on_confirmTransferBtn_clicked() {
    if (!confirmWithPassword()) return;

    int selectedAccIndex = ui->transferFromCombo->currentIndex();
    if (selectedAccIndex == -1) return;

    string targetPhone = ui->transferPhoneEdit->text().toStdString();
    long double amount = ui->amountEdit->text().toDouble();


    int receiverIndex = findUserByPhone(targetPhone, users);


    if (receiverIndex == currentUserIndex) {
        QMessageBox::warning(this, "Transfer Error", "You cannot transfer to yourself via phone number!");
        return;
    }

    if (receiverIndex == -1) {
        QMessageBox::warning(this, "Transfer Error", "Receiver phone number not found!");
        return;
    }

    account &senderAcc = users[currentUserIndex].accounts[selectedAccIndex];

    account &receiverAcc = users[receiverIndex].accounts[0];

    if (senderAcc.balance >= amount && amount > 0) {

        transferTo(senderAcc, receiverAcc, amount);


        recordTransaction(senderAcc, "Me", users[receiverIndex].username, amount);


        recordTransaction(receiverAcc, users[currentUserIndex].username, "Me", amount);

        QMessageBox::information(this, "Success", "Transfer Completed from " + QString::fromStdString(senderAcc.bankName)+getTransactionTime());
        ui->stackedWidget->setCurrentIndex(DASHBOARD_PAGE);
    } else {
        QMessageBox::critical(this, "Error", "Insufficient Balance or Invalid Amount!");
    }
}

void MainWindow::on_backToDashBtn_clicked() {
    ui->stackedWidget->setCurrentIndex(DASHBOARD_PAGE);
}

void MainWindow::on_historyBtn_clicked() {
    ui->historyList->setWordWrap(true);
    ui->stackedWidget->setCurrentIndex(HISTORY_PAGE);
    ui->accComboBox->clear();
    ui->historyList->clear();

    for (int i = 0; i < users[currentUserIndex].num_of_accounts; i++) {
        string bankName = users[currentUserIndex].accounts[i].bankName;
        ui->accComboBox->addItem(QString::fromStdString(bankName));
    }
    if (users[currentUserIndex].num_of_accounts == 0) {
        ui->accComboBox->addItem("No accounts linked");
    }
}


void MainWindow::on_accComboBox_currentIndexChanged(int index) {
    ui->historyList->clear();
    if (index >= 0 && index < users[currentUserIndex].num_of_accounts) {
        account &selectedAcc = users[currentUserIndex].accounts[index];
        for (int i = 0; i < selectedAcc.transaction_count; i++) {
            string record = "From: " + selectedAcc.trans[i].from +
                            " | To: " + selectedAcc.trans[i].to +
                            " | Amount: $" + to_string((double)selectedAcc.trans[i].amount);
            ui->historyList->addItem(QString::fromStdString(record));
        }
        if (selectedAcc.transaction_count == 0) {
            ui->historyList->addItem("No transactions for this account.");
        }
    }
}

void MainWindow::on_backToDashBtn_1_clicked() {
    ui->stackedWidget->setCurrentIndex(DASHBOARD_PAGE);
}

void MainWindow::on_donationBtn_clicked() {
    if (currentUserIndex != -1) {
        ui->donationAccCombo->clear();
        for (int i = 0; i < users[currentUserIndex].num_of_accounts; i++) {
            ui->donationAccCombo->addItem(QString::fromStdString(users[currentUserIndex].accounts[i].bankName));
        }
        ui->stackedWidget->setCurrentIndex(DONATION_PAGE);
    }
}

void MainWindow::on_donationSubmitBtn_clicked() {
     if (!confirmWithPassword()) return;

    int selectedAccIndex = ui->donationAccCombo->currentIndex();
    if (selectedAccIndex == -1) {
        QMessageBox::warning(this, "Selection Error", "Please select a bank account first.");
        return;
    }

    double amount = ui->donationAmountEdit->text().toDouble();
    QString organization = ui->charityCombo->currentText();

    account &userAcc = users[currentUserIndex].accounts[selectedAccIndex];

    if (amount > 0 && userAcc.balance >= (long double)amount) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Confirm Donation",
                                      "Are you sure you want to donate $" + QString::number(amount) +
                                          " from your " + QString::fromStdString(userAcc.bankName) + " account?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            userAcc.balance -= (long double)amount;
            recordTransaction(userAcc, "Me", "Donation to " + organization.toStdString(), (long double)amount);

            QStringList thankYouMessages;
            thankYouMessages << "Thank you for your generosity! Your donation will make a real difference.";
            thankYouMessages << "Transaction Successful! Thank you for being part of the change.";
            thankYouMessages << "Donation Accepted! Your kindness is truly appreciated.";
            thankYouMessages << "Great Job! You've successfully contributed to a noble cause.";

            QString randomMsg = thankYouMessages.at(rand() % thankYouMessages.size());

            QMessageBox msgBox;
            msgBox.setWindowTitle("Donation Successful"+getTransactionTime());
            msgBox.setText(randomMsg);
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setStyleSheet("QLabel{ min-width: 350px; color: #1a73e8; font-weight: bold; }");
            msgBox.exec();
            ui->donationAmountEdit->clear();
            ui->stackedWidget->setCurrentIndex(DASHBOARD_PAGE);
        }
    } else {
        QMessageBox::critical(this, "Insufficient Funds",
                              "Sorry, your " + QString::fromStdString(userAcc.bankName) +
                                  " account doesn't have enough balance for this donation.");
    }
}

void MainWindow::on_backToDashBtn_2_clicked() {
    ui->stackedWidget->setCurrentIndex(DASHBOARD_PAGE);
}

void MainWindow::on_Newaccount_clicked() {
    ui->stackedWidget->setCurrentIndex(ADD_ACCOUNT_PAGE);
}

void MainWindow::on_confirmAddAccBtn_clicked() {
    if (currentUserIndex == -1) return;

    if (users[currentUserIndex].num_of_accounts >= MAX_ACCOUNTS) {
        QMessageBox::warning(this, "Limit Reached",
                             "You have reached the maximum limit of " + QString::number(MAX_ACCOUNTS) + " accounts.");
        return;
    }
    QString qBank = ui->bankNameEdit->text().trimmed();
    QString qCard = ui->cardNumberEdit->text().trimmed();
    QString qCvv  = ui->cvvEdit->text().trimmed();
    QString qExp  = ui->expiryDateEdit->text().trimmed();

    string bName = ui->bankNameEdit->text().toStdString();
    string cNo   = ui->cardNumberEdit->text().toStdString();
    string cvv   = ui->cvvEdit->text().toStdString();
    string exp   = ui->expiryDateEdit->text().toStdString();

    bool isInputEmpty = (bName.empty() || cNo.empty() || cvv.empty() || exp.empty());
    bool isLogicValid = isValidAccount(cNo, cvv, exp);

    if (isInputEmpty || !isLogicValid) {
        QMessageBox::critical(this, "Invalid Data",
                              "Please check your entries:\n"
                              "- Bank Name cannot be empty.\n"
                              "- Card Number must be 16 digits.\n"
                              "- CVV must be 3 digits.\n"
                              "- Expiry Date must be in MM/YY format (e.g., 12/26).");
        return;
    }

    int accIdx = users[currentUserIndex].num_of_accounts;
    account &newAcc = users[currentUserIndex].accounts[accIdx];

    newAcc.bankName = bName;
    newAcc.cardNo = cNo;
    newAcc.cvvCode = cvv;
    newAcc.expirationDate = exp;
    newAcc.balance = 0.0;
    newAcc.transaction_count = 0;

    recordTransaction(newAcc, "System", "Account Created", 0.0);
    users[currentUserIndex].num_of_accounts++;

    QMessageBox::information(this, "Success", "Bank account linked!");


    ui->bankNameEdit->clear();
    ui->cardNumberEdit->clear();
    ui->cvvEdit->clear();
    ui->expiryDateEdit->clear();


    ui->stackedWidget->setCurrentIndex(DASHBOARD_PAGE);
}


void MainWindow::on_backToDashBtn_3_clicked() {
    ui->stackedWidget->setCurrentIndex(DASHBOARD_PAGE);
}

void MainWindow::on_viewBalanceBtn_clicked() {
    ui->balanceAccCombo->clear();

    for(int i = 0; i < users[currentUserIndex].num_of_accounts; ++i) {
        ui->balanceAccCombo->addItem(QString::fromStdString(users[currentUserIndex].accounts[i].bankName));
    }


    ui->stackedWidget->setCurrentIndex(VIEW_BALANCE);
}

void MainWindow::on_balanceAccCombo_currentIndexChanged(int index) {
    account &selectedAcc = users[currentUserIndex].accounts[index];


    QString info = "Bank Name: " + QString::fromStdString(selectedAcc.bankName) + "\n";


    info += "Account No: " + QString::fromStdString(selectedAcc.cardNo) + "\n";

    info += "Current Balance: " + QString::number((double)selectedAcc.balance, 'f', 2) + " EGP";

    ui->balanceDisplayLabel->setText(info);
}

void MainWindow::on_backToDashBtn_4_clicked() {
    ui->stackedWidget->setCurrentIndex(DASHBOARD_PAGE);
}

void MainWindow::on_viewDepositBtn_clicked() {
    ui->depositAccCombo->clear();
    for(int i = 0; i < users[currentUserIndex].num_of_accounts; ++i) {
        ui->depositAccCombo->addItem(QString::fromStdString(users[currentUserIndex].accounts[i].bankName));
    }
    ui->stackedWidget->setCurrentIndex(DEPOSIT);
}

void MainWindow::on_depositSubmitBtn_clicked() {
     if (!confirmWithPassword()) return;

    int selectedIndex = ui->depositAccCombo->currentIndex();
    if (selectedIndex == -1) return;

    double amount = ui->depositAmountEdit->text().toDouble();

    if (amount > 0) {
        account &targetAcc = users[currentUserIndex].accounts[selectedIndex];
        targetAcc.balance += (long double)amount;

        recordTransaction(targetAcc, "CASH", "SELF", (long double)amount);

        QMessageBox::information(this, "Deposit Successful"+getTransactionTime(),
                                 "Amount of $" + QString::number(amount) +
                                     " has been added to " + QString::fromStdString(targetAcc.bankName));

        ui->depositAmountEdit->clear();


        ui->stackedWidget->setCurrentIndex(DASHBOARD_PAGE);
    } else {
        QMessageBox::warning(this, "Invalid Amount", "Please enter a valid amount greater than 0.");
    }
}

void MainWindow::on_backToDashBtn_5_clicked() {
    ui->stackedWidget->setCurrentIndex(DASHBOARD_PAGE);
}

void MainWindow::on_logoutBtn_clicked() {
    currentUserIndex = -1;
    ui->UserEdit->clear();
    ui->PassEdit->clear();
    ui->PassEdit->setText("");
    ui->stackedWidget->setCurrentIndex(WELCOME_PAGE);
}

QString MainWindow::getTransactionTime() {
    return QLocale(QLocale::English).toString(QDateTime::currentDateTime(), "dddd, MMMM dd, yyyy, hh:mm:ss AP");
};