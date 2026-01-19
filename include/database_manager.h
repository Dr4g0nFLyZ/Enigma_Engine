#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QString>
#include <string>

class DatabaseManager {
public:
   DatabaseManager(const QString& path) {
      db = QSqlDatabase::addDatabase("QSQLITE");
      db.setDatabaseName(path);
   }

   bool open() {
      if (!db.open()) {
         qCritical() << "Error: Connection with database failed:" << db.lastError().text();
         return false;
      }

      qDebug() << "SQLite Database connected successfully!";
      return createTable();
   }

   bool logMessage(const std::string& message) {
      QSqlQuery query;
      query.prepare("INSERT INTO logs (msg) VALUES (:msg)");
      query.bindValue(":msg", QString::fromStdString(message));

      if (!query.exec()) {
         qWarning() << "Insert failed:" << query.lastError().text();
         return false;
      }
      return true;
   }

private:
   QSqlDatabase db;

   bool createTable() {
      QSqlQuery query;
      return query.exec("CREATE TABLE IF NOT EXISTS logs (id INTEGER PRIMARY KEY, msg TEXT)");
   }
};

#endif
