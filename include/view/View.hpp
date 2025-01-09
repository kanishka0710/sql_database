#ifndef View_h
#define View_h

#include <iostream>
#include <functional>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include "TabularView.hpp"

namespace ECE141 {

    // Completely generic view, which you will subclass to show information
    class View {
    public:
        virtual ~View() {}
        virtual bool show(std::ostream &aStream) = 0;
    };

    using ViewListener = std::function<void(View &aView)>;

    class TextView : public View {
        std::string message;
    public:
        TextView(const std::string &aMessage) : message(aMessage) {}

        bool show(std::ostream &aStream) override {
            aStream << message;
            return true;
        }
    };

    class DatabaseView : public View {
        std::vector<std::string> databaseFiles;
        std::string message;
    public:
        DatabaseView(std::string &aMessage, std::vector<std::string>& files) : message(aMessage), databaseFiles(files) {}

        bool show(std::ostream &aStream) override {
            if (databaseFiles.empty()) {
                aStream << message;
            } else {
                TabularView aView(aStream, databaseFiles);
                aView.show();
            }
            return true;
        }
    };


    class TableView2 : public View {
    private:
        std::vector<std::string> columns;
        std::vector<std::vector<std::string>> data;
        std::vector<size_t> columnWidths;
        Timer timer; // Timer object
        bool success; // Indicates whether the operation was successful or failed
        int rowCount;
        bool isTable;

        void calculateColumnWidths() {
            columnWidths.resize(columns.size());
            for (size_t i = 0; i < columns.size(); ++i) {
                columnWidths[i] = columns[i].length(); // Start with header width
                for (auto& row : data) {
                    if (i < row.size()) {
                        columnWidths[i] = std::max(columnWidths[i], row[i].length());
                    }
                }
            }
        }

        void printSeparator(std::ostream &aStream) {
            aStream << "+";
            for (size_t width : columnWidths) {
                aStream << std::string(width + 2, '-') << "+";
            }
            aStream << std::endl;
        }

        void printRow(const std::vector<std::string>& row, std::ostream &aStream) {
            aStream << "|";
            for (size_t i = 0; i < row.size(); ++i) {
                aStream << " " << std::left << std::setw(columnWidths[i]) << row[i] << " |";
            }
            aStream << std::endl;
        }

        std::string generateFinalMessage() const {
            if(!isTable){
                return success ?
                "Query OK, " + std::to_string((data.empty() ? (rowCount==-1 ? 1 : rowCount) : data.size())) + " rows affected (" + std::to_string(timer.elapsed()) + " sec)"
                        : "Query FAIL Validation, 0 rows affected (" + std::to_string(timer.elapsed()) + " sec)";
            } else{
                return std::to_string(data.size()) + " rows in set (" + std::to_string(timer.elapsed()) + " sec)";
            }

        }

    public:
        TableView2() : success(true), rowCount(-1) {
            timer = Config::getTimer();
        }

        ~TableView2() = default;

        void setHeaders(const std::vector<std::string>& headers) {
            columns = headers;
            calculateColumnWidths();
        }

        void setData(const std::vector<std::vector<std::string>>& content) {
            data = content;
            calculateColumnWidths();
        }

        void setCount(const int& count) {
            rowCount = count;
        }

        void setIfTable(bool table){
            isTable=table;
        }

        bool show(std::ostream &aStream) override {
            if (!success || (columns.empty() && data.empty())) {
                aStream << generateFinalMessage() << std::endl;  // Print only the final message
                return true;
            }

            printSeparator(aStream);
            printRow(columns, aStream);
            printSeparator(aStream);
            for (size_t i = 0; i < data.size(); ++i) {
                printRow(data[i], aStream);
                if (i < data.size() - 1) {
                    printSeparator(aStream);  // Only print a separator between rows, not after the last one
                }
            }
            printSeparator(aStream);  // Final separator at the end of the table
            aStream << generateFinalMessage() << std::endl;  // Print the final message
            return true;
        }

        void setSuccess(bool aSuccess) {
            success = aSuccess;
        }


    };

    class TableView : public View {
    private:
        std::vector<std::string> columns;
        std::vector<std::vector<std::string>> data;
        std::vector<size_t> columnWidths;

        void calculateColumnWidths() {
            columnWidths.resize(columns.size());
            for (size_t i = 0; i < columns.size(); ++i) {
                columnWidths[i] = columns[i].length(); // Start with header width
                for (auto& row : data) {
                    if (i < row.size()) {
                        columnWidths[i] = std::max(columnWidths[i], row[i].length());
                    }
                }
            }
        }

        void printSeparator(std::ostream &aStream) {
            aStream << "+";
            for (size_t width : columnWidths) {
                aStream << std::string(width + 2, '-') << "+";
            }
            aStream << std::endl;
        }

        void printRow(const std::vector<std::string>& row, std::ostream &aStream) {
            aStream << "|";
            for (size_t i = 0; i < row.size(); ++i) {
                aStream << " " << std::left << std::setw(columnWidths[i]) << row[i] << " |";
            }
            aStream << std::endl;
        }

    public:
        TableView(const std::vector<std::string>& headers, const std::vector<std::vector<std::string>>& content)
                : columns(headers), data(content) {
            calculateColumnWidths();
        }

        virtual bool show(std::ostream &aStream) override {
            printSeparator(aStream);
            printRow(columns, aStream);
            printSeparator(aStream);
            for (size_t i = 0; i < data.size(); ++i) {
                printRow(data[i], aStream);
                if (i < data.size() - 1) {
                    printSeparator(aStream);  // Only print a separator between rows, not after the last one
                }
            }
            printSeparator(aStream);  // Final separator at the end of the table
            return true;
        }
    };
}

#endif /* View_h */
