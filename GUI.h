#ifndef GUI_H
#define GUI_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDialog>
#include <QComboBox>
#include <QLabel>
#include "Database.h"

class GUI : public QMainWindow {
    Q_OBJECT

public:
    GUI(QWidget *parent = nullptr);

private slots:
    void onCreateDB();
    void onOpenDB();
    void onAddRecord();
    void onSearch();
    void onDelete();
    void onEdit();
    void onBackup();
    void onRestore();
    void refreshTable();

private:
    Database db;
    QTableWidget *table;
    QLineEdit *idInput;
    QLineEdit *nameInput;
    QLineEdit *courseInput;
    QLineEdit *gradeInput;
    QComboBox *searchFieldCombo;
    QLineEdit *searchValueInput;
};

#endif