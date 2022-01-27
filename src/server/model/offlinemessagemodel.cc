#include "offlinemessagemodel.h"
#include "server/db/mysqlpool.h"


bool OfflineMessageModel::insert(int id, const std::string& message)
{
    char sql[1024] = {0}; 
    snprintf(sql, 1024, "insert into offlineMessage(userid, message) values('%d', '%s')", id, message.c_str());

    std::shared_ptr<MySql> conn = MySqlPool::getMysqlPool()->getConnect();
      
    return conn->update(sql);
}

bool OfflineMessageModel::remove(int id)
{
    char sql[1024] = {0};
    snprintf(sql, 1024, "delete from offlineMessage where userid = '%d'", id);

    std::shared_ptr<MySql> conn = MySqlPool::getMysqlPool()->getConnect();

    return conn->update(sql);
}

std::vector<std::string> OfflineMessageModel::query(int id)
{
    char sql[1024] = {0};
    snprintf(sql, 1024, "select * from offlineMessage where userid = '%d'", id);

    std::shared_ptr<MySql> conn = MySqlPool::getMysqlPool()->getConnect();  

    MYSQL_RES *result = conn->query(sql);
    if (result == nullptr) {
        return {};
    }

    
    std::vector<std::string> messvec;
    MYSQL_ROW row;
    while (row = mysql_fetch_row(result)) {
        messvec.push_back(std::string(row[1]));
    }
    mysql_free_result(result);

    return messvec;
}
