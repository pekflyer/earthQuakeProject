//
//  mysql_connector.cpp
//  earthQuakeProject
//
//  Created by Allan Yong on 12-06-08.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "mysql_connector.h"

using namespace std;

mysql_connector::mysql_connector()
{
    try {
        ::sql::Driver *driver;
        ::sql::Connection *con;
        ::sql::Statement *stmt;
        ::sql::ResultSet *res;
        driver = ::sql::mysql::get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", "root", "xiaofeng");
        con->setSchema("earthquake");
        stmt= con->createStatement();
        res = stmt->executeQuery("SELECT * FROM `earthQuake` WHERE gender='男'");
        while(res->next())
        {
            cout << "\t... MySQL replies: ";
            /* Access column data by alias or column name */
            cout << res->getString("gender") << endl;
            cout << "\t... MySQL says it again: ";
            /* Access column fata by numeric offset, 1 is the first column */
            cout << res->getString(1) << endl;

        }
        delete res;
        delete stmt;
        delete con;
        
    } catch (sql::SQLException &e) {
            
        cout << "# ERR: SQLException in " << __FILE__;
        cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
        cout << "# ERR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << " )" << endl;
    }
    cout <<endl;

    //return EXIT_SUCCESS;
}