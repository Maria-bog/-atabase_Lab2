#include "GUI.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>

GUI::GUI(QWidget *parent) : QMainWindow(parent) {
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout();

    table = new QTableWidget();
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"ID","Name","Active","AvgGrade","Course"});
    table->horizontalHeader()->setStretchLastSection(true);

    QHBoxLayout *form = new QHBoxLayout();
    idInput = new QLineEdit(); idInput->setPlaceholderText("ID");
    nameInput = new QLineEdit(); nameInput->setPlaceholderText("Name");
    gradeInput = new QLineEdit(); gradeInput->setPlaceholderText("Average Grade");
    courseInput = new QLineEdit(); courseInput->setPlaceholderText("Course");

    form->addWidget(idInput);
    form->addWidget(nameInput);
    form->addWidget(gradeInput);
    form->addWidget(courseInput);

    QHBoxLayout *searchForm = new QHBoxLayout();
    searchFieldCombo = new QComboBox();
    searchFieldCombo->addItems({"id", "name", "cours", "averageGrade"});

    searchValueInput = new QLineEdit();
    searchValueInput->setPlaceholderText("Search value");

    searchForm->addWidget(new QLabel("Search by:"));
    searchForm->addWidget(searchFieldCombo);
    searchForm->addWidget(searchValueInput);

    QHBoxLayout *buttons = new QHBoxLayout();
    QPushButton *createBtn = new QPushButton("Create DB");
    QPushButton *openBtn = new QPushButton("Open DB");
    QPushButton *addBtn = new QPushButton("Add Record");
    QPushButton *searchBtn = new QPushButton("Search");
    QPushButton *deleteBtn = new QPushButton("Delete");
    QPushButton *editBtn = new QPushButton("Edit");
    QPushButton *backupBtn = new QPushButton("Backup");
    QPushButton *restoreBtn = new QPushButton("Restore");

    buttons->addWidget(createBtn);
    buttons->addWidget(openBtn);
    buttons->addWidget(addBtn);
    buttons->addWidget(searchBtn);
    buttons->addWidget(deleteBtn);
    buttons->addWidget(editBtn);
    buttons->addWidget(backupBtn);
    buttons->addWidget(restoreBtn);

    connect(createBtn, &QPushButton::clicked, this, &GUI::onCreateDB);
    connect(openBtn, &QPushButton::clicked, this, &GUI::onOpenDB);
    connect(addBtn, &QPushButton::clicked, this, &GUI::onAddRecord);
    connect(searchBtn, &QPushButton::clicked, this, &GUI::onSearch);
    connect(deleteBtn, &QPushButton::clicked, this, &GUI::onDelete);
    connect(editBtn, &QPushButton::clicked, this, &GUI::onEdit);
    connect(backupBtn, &QPushButton::clicked, this, &GUI::onBackup);
    connect(restoreBtn, &QPushButton::clicked, this, &GUI::onRestore);

    mainLayout->addLayout(form);
    mainLayout->addLayout(searchForm);
    mainLayout->addLayout(buttons);
    mainLayout->addWidget(table);

    central->setLayout(mainLayout);

    setWindowTitle("Student Database");
    resize(800, 600);
}

void GUI::refreshTable() {
    table->setRowCount(0);
    auto all = db.getAll();

    for (size_t i = 0; i < all.size(); i++) {
        table->insertRow(i);
        table->setItem(i, 0, new QTableWidgetItem(QString::number(all[i].id)));
        table->setItem(i, 1, new QTableWidgetItem(all[i].name));
        table->setItem(i, 2, new QTableWidgetItem(all[i].isActive ? "1" : "0"));
        table->setItem(i, 3, new QTableWidgetItem(QString::number(all[i].averageGrade)));
        table->setItem(i, 4, new QTableWidgetItem(QString::number(all[i].cours)));
    }
}

void GUI::onCreateDB() {
    QString path = QFileDialog::getSaveFileName(this, "Create DB", "", "DB Files (*.db)");
    if(path.isEmpty()) return;

    if(db.create(path.toStdString())) {
        refreshTable();
        QMessageBox::information(this, "Success", "Database created successfully.");
    } else {
        QMessageBox::warning(this, "Error", "Failed to create database.");
    }
}

void GUI::onOpenDB() {
    QString path = QFileDialog::getOpenFileName(this, "Open DB", "", "DB Files (*.db)");
    if(path.isEmpty()) return;

    if(db.open(path.toStdString())) {
        refreshTable();
        QMessageBox::information(this, "Success", "Database opened successfully.");
    } else {
        QMessageBox::warning(this, "Error", "Failed to open database.");
    }
}

void GUI::onAddRecord() {
    Student st;

    if (idInput->text().isEmpty()) {
        QMessageBox::warning(this, "Error", "ID cannot be empty.");
        return;
    }

    st.id = idInput->text().toInt();

    std::string nameStr = nameInput->text().toStdString();
    snprintf(st.name, sizeof(st.name), "%s", nameStr.c_str());

    st.averageGrade = gradeInput->text().toDouble();
    st.cours = courseInput->text().toInt();
    st.isActive = true;

    std::string err;
    if(db.addRecord(st, err)) {
        refreshTable();
        idInput->clear();
        nameInput->clear();
        gradeInput->clear();
        courseInput->clear();
        QMessageBox::information(this, "Success", "Record added successfully.");
    } else {
        QMessageBox::warning(this, "Error", QString::fromStdString(err));
    }
}

void GUI::onSearch() {
    QString field = searchFieldCombo->currentText();
    QString value = searchValueInput->text();

    if (value.isEmpty()) {
        QMessageBox::warning(this, "Search", "Please enter search value.");
        return;
    }

    auto results = db.searchByField(field.toStdString(), value.toStdString());

    if(results.empty()) {
        QMessageBox::information(this, "Search", "No records found.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Search Results");
    dialog.resize(600, 300);

    QVBoxLayout layout(&dialog);
    QTableWidget tableR;
    tableR.setColumnCount(5);
    tableR.setHorizontalHeaderLabels({"ID","Name","Active","AvgGrade","Course"});
    tableR.setRowCount(results.size());

    for (size_t i = 0; i < results.size(); i++) {
        tableR.setItem(i, 0, new QTableWidgetItem(QString::number(results[i].id)));
        tableR.setItem(i, 1, new QTableWidgetItem(results[i].name));
        tableR.setItem(i, 2, new QTableWidgetItem(results[i].isActive ? "1" : "0"));
        tableR.setItem(i, 3, new QTableWidgetItem(QString::number(results[i].averageGrade)));
        tableR.setItem(i, 4, new QTableWidgetItem(QString::number(results[i].cours)));
    }

    tableR.horizontalHeader()->setStretchLastSection(true);
    layout.addWidget(&tableR);

    QPushButton closeBtn("Close");
    layout.addWidget(&closeBtn);
    connect(&closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();
}

void GUI::onDelete() {
    if (idInput->text().isEmpty()) {
        QMessageBox::warning(this, "Delete", "Enter ID to delete.");
        return;
    }

    int id = idInput->text().toInt();

    size_t deleted = db.deleteByField("id", std::to_string(id));

    if(deleted > 0) {
        refreshTable();
        idInput->clear();
        QMessageBox::information(this, "Success", "Record deleted.");
    } else {
        QMessageBox::warning(this, "Error", "Record not found.");
    }
}

void GUI::onEdit() {
    if (idInput->text().isEmpty()) {
        QMessageBox::warning(this, "Error", "Enter ID to edit.");
        return;
    }

    Student st;

    st.id = idInput->text().toInt();

    snprintf(st.name, sizeof(st.name), "%s", nameInput->text().toStdString().c_str());
    st.averageGrade = gradeInput->text().toDouble();
    st.cours = courseInput->text().toInt();
    st.isActive = true;

    if(db.editRecordByKey(st.id, st)) {
        refreshTable();
        idInput->clear();
        nameInput->clear();
        gradeInput->clear();
        courseInput->clear();
        QMessageBox::information(this, "Success", "Record updated.");
    } else {
        QMessageBox::warning(this, "Error", "Failed to update record.");
    }
}


void GUI::onBackup() {
    QString path = QFileDialog::getSaveFileName(this, "Backup DB", "", "DB Files (*.db);;All Files (*)");
    if(path.isEmpty()) return;
    
    if(!path.endsWith(".db", Qt::CaseInsensitive)) {
        path += ".db";
    }

    if(db.backup(path.toStdString())) {
        QMessageBox::information(this, "Success", "Backup created.");
    } else {
        QMessageBox::warning(this, "Error", "Backup failed.");
    }
}

void GUI::onRestore() {
    QString path = QFileDialog::getOpenFileName(this, "Restore DB", "", "DB Files (*.db);;Backup Files (*.db *.backup);;All Files (*)");
    if(path.isEmpty()) return;

    std::cout << "Attempting to restore from: " << path.toStdString() << std::endl;

    if(db.restoreFromBackup(path.toStdString())) {
        refreshTable(); 
        QMessageBox::information(this, "Success", "Database restored successfully.");
    } else {
        QMessageBox::warning(this, "Error", "Restore failed. Check console for details.");
    }
}