#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:

    void on_confirmTransferBtn_clicked();

    void on_loginBtn_clicked();

    void on_SignUpBtn_clicked();

    void on_toSignUpBtn_clicked();

    void on_toLogInBtn_clicked();

    void on_historyBtn_clicked();

    void on_accComboBox_currentIndexChanged(int index);

    void on_donationSubmitBtn_clicked();

    void on_donationBtn_clicked();

    void on_confirmAddAccBtn_clicked();

    void on_Newaccount_clicked();

    void on_backToDashBtn_clicked();

    void on_backToDashBtn_1_clicked();

    void on_backToDashBtn_2_clicked();

    void on_backToDashBtn_3_clicked();

    void on_backToDashBtn_4_clicked();

    void on_backToDashBtn_5_clicked();

    void on_logoutBtn_clicked();

    void on_welcomeLoginBtn_clicked();

    void on_welcomeSignupBtn_clicked();

    void on_welcomeExitBtn_clicked();

    void on_TransferBtn_clicked();

    void on_viewBalanceBtn_clicked();

    void on_balanceAccCombo_currentIndexChanged(int index);

    void on_viewDepositBtn_clicked();

    void on_depositSubmitBtn_clicked();

  //  void updateDashboard();

  //  void showProcessTime();
    QString  getTransactionTime();
    void loadDataFromFile();

   void  saveDataToFile();

   bool confirmWithPassword();


private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
