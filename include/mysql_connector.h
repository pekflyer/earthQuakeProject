//
//  mysql_connector.h
//  earthQuakeProject
//
//  Created by Allan Yong on 12-06-08.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef earthQuakeProject_mysql_connector_h
#define earthQuakeProject_mysql_connector_h
//#include <mysql.h> 
#include "cinder/app/AppBasic.h"
#include "mysql_driver.h"
#include "mysql_connection.h"
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <iostream>
#include <vector>


using namespace ci;
using namespace std;



class mysql_connector
{
public:
    mysql_connector();
    int mysql_connect();
    vector<string> & getNameString();
    vector<string> & getExtraString();
    vector<string>          nameStrings;
    vector<string>          extraInfoStrings;
private:
    ::sql::Driver *driver;
    ::sql::Connection *con;
    ::sql::Statement *stmt;
    ::sql::ResultSet *res;

};

#endif
