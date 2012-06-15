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
#include <stdio.h>
#include <stdexcept>
#include "mysql_connector.h"
#include "boost/date_time/gregorian/gregorian.hpp"
#include "time.h"
#include <set>
#include <algorithm>

using namespace std;
using namespace boost::gregorian;
mysql_connector::mysql_connector()
{
    
}

//int mysql_connector::mysql_connect()
//{
//    mysql_init(&mysql);
//    if(!mysql_real_connect(&mysql, "127.0.0.1", "root", "xiaofeng", "earthquake", 3306, NULL, 0))
//       {}
//       else
//       {
//       
//       }
//    return(0);
//}

int mysql_connector::mysql_connect()
{
    cout << endl;
    
    date dt= day_clock::local_day();
    std::string dy = to_iso_string(dt);
    std::string dd = to_iso_string(dt);
    dy.erase(4,8);
    dd.erase(0,4);
   

    try {
       
         std::cout << dd << std::endl;
        driver = ::sql::mysql::get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", "root", "xiaofeng");
        con->setSchema("earthquake");
        stmt= con->createStatement();
        res = stmt->executeQuery("SELECT * FROM `earthQuake` WHERE birth LIKE '%" + dd + "'");
        //res = stmt->executeQuery("SELECT * FROM `earthQuake`");

        
        while(res->next())
        {
            cout << "\t... MySQL replies: ";
            /* Access column data by alias or column name */
            cout << res->getString("name") << endl;
            cout << "\t... MySQL says it again: ";
            /* Access column fata by numeric offset, 1 is the first column */
            cout << res->getString(1) << endl;
           
            //Victim's Name
            std::string name = res->getString("name");
           nameStrings.push_back(name);
        
            //Birth Year Calculation
            std::string tempBirthYear = res->getString("birth");
            std::string tempMonth, tempDay;
            tempMonth = tempBirthYear;
            tempDay  = tempBirthYear;
            tempBirthYear.erase(4,8);
            int intBirthYear = atoi(tempBirthYear.c_str());
            int currYear = atoi(dy.c_str());
            int currAge = currYear - intBirthYear;  
            int deathAge = 2008 - intBirthYear;

            tempMonth.erase(0,4);
            tempMonth.erase(2,4);
            tempDay.erase(0,6);
            
            stringstream extraInfo;
            extraInfo << res->getString("gender") << ", " << intBirthYear << "年" << tempMonth << "月" << tempDay << "日, " << deathAge << "岁" << " - 2012年" << tempMonth << "月" << tempDay << "日, " << currAge << "岁" <<  endl;
            
            cout << extraInfo.str() << endl;
            extraInfoStrings.push_back(extraInfo.str());
     
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
        return EXIT_FAILURE;
    }
    cout <<endl;

 return EXIT_SUCCESS;
}

vector<string> & mysql_connector::getNameString()
{
    vector<string> &temp = nameStrings;
    return temp;
}

vector<string> & mysql_connector::getExtraString()
{
    
    vector<string> &temp = extraInfoStrings;
    return temp;
}