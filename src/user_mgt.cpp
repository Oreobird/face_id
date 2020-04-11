#include "base_convert.h"
#include "base_string.h"
#include "base_logger.h"
#include "base_xml_parser.h"
#include <sstream>
#include "user_mgt.h"
#include "mysql.h"
#include "mysql_conn.h"
#include "mysql_mgt.h"

extern mysql_mgt g_mysql_mgt;
extern Logger g_logger;
User_Mgt *g_user_mgt = NULL;

#define uint64 unsigned long long

#define CHECK_RET(nRet)                                                      \
{                                                                     		 \
		if (nRet != 0)                                                       \
		{                                                                    \
			XCP_LOGGER_ERROR(&g_logger, "check return failed: %d.\n", nRet); \
			return nRet;                                                     \
		}                                                                    \
}

#define CHECK_RET_ROLL_BACK(nRet, conn)                                      \
{                                                                     		 \
		if (nRet != 0)                                                       \
		{                                                                    \
			XCP_LOGGER_ERROR(&g_logger, "check return failed: %d.\n", nRet); \
			conn->rollback();                                                \
			conn->autocommit(true);                                          \
			return nRet;                                                     \
		}                                                                    \
}

#define MYSQL_GET_CONN_THEN_LOCK(conn)                           \
	if (!g_mysql_mgt.get_conn(conn))                             \
	{                                                            \
		XCP_LOGGER_ERROR(&g_logger, "get mysql conn failed.\n"); \
		errInfo = "get mysql conn failed";						 \
		return ERR_GET_MYSQL_CONN_FAILED;                        \
	}                                                            \
	MySQL_Guard guard_mysql(conn);

User_Mgt::User_Mgt()
{
}

User_Mgt::~User_Mgt()
{
}

void User_Mgt::dump_feat_vec()
{
	for (auto user:m_users)
	{
		dbg("user: %s\nfeat:", user.name.c_str());
		for (auto feat: user.feat_vec)
		{
			dbg("%f ", feat);
		}
		dbg("\n");
	}
}

int User_Mgt::init(void)
{
	// Load users in db
	std::string errInfo;
	mysql_conn_Ptr conn;
	MYSQL_GET_CONN_THEN_LOCK(conn);

	MySQL_Row_Set row_set;
	std::ostringstream sql;

	sql << "select * from tbl_user;";

	int nRet = conn->query_sql(sql.str(), row_set, errInfo);
	CHECK_RET(nRet);

	for (int i = 0; i < row_set.row_count(); i++)
	{
		std::vector<float> feat = str_to_feat_vec(row_set[i][2]);

		user_t user;
		user.id = row_set[i][0];
		user.name = row_set[i][1];
		user.feat_vec.assign(feat.begin(), feat.end());

		m_users.push_back(user);
	}

	//dump_feat_vec();
	return 0;
}

std::vector<user_t> *User_Mgt::get_users(void)
{
	Thread_Mutex_Guard guard(_mutex);
	return &m_users;
}

std::string User_Mgt::feat_vec_to_str(std::vector<float> &feat_vec)
{
	std::ostringstream feat_stream;
	std::vector<float>::iterator it;
	std::string delim = ",";

	for (it = feat_vec.begin(); it != feat_vec.end() - 1; ++it)
	{
		feat_stream << (*it) << delim;
	}

	feat_stream << (*it);

	return feat_stream.str();
}

std::vector<float> User_Mgt::str_to_feat_vec(const std::string &feat_vec_str)
{
	std::string delim = ",";
    int last = 0;
	std::vector<float> feat_vec;
    int index = feat_vec_str.find_first_of(delim, last);

    while (index != std::string::npos)
    {
    	std::istringstream feat_stream(feat_vec_str.substr(last, index - last));
		float feat;
	    feat_stream >> feat;

        feat_vec.push_back(feat);
        last = index + 1;
        index = feat_vec_str.find_first_of(delim, last);
    }

    if (index < 0 && feat_vec_str.length() - last > 0)
    {
        std::istringstream feat_stream(feat_vec_str.substr(last, feat_vec_str.length() - last));
		float feat;
	    feat_stream >> feat;
    	feat_vec.push_back(feat);
	}

    return feat_vec;
}

bool User_Mgt::user_exist_in_db(const std::string &user_id)
{
	std::string errInfo;

	mysql_conn_Ptr conn;
	MYSQL_GET_CONN_THEN_LOCK(conn);

	std::ostringstream sql;
	sql << "select * from tbl_user where id = '"
		<< user_id << "';";

	MySQL_Row_Set row_set;

	int nRet = conn->query_sql(sql.str(), row_set, errInfo);
	if (nRet != 0)
	{
		XCP_LOGGER_ERROR(&g_logger, "check return failed: %d.\n", nRet);
	}

	return row_set.row_count() > 0 ? true : false;
}

bool User_Mgt::user_exist(const std::string &user_id)
{
	bool exist = false;

	for (std::vector<user_t>::iterator it = m_users.begin(); it != m_users.end(); ++it)
	{
		if ((*it).id == user_id)
		{
			exist = true;
		}
	}

	return exist;
}

int User_Mgt::add_user(user_t &user, const std::string img_path)
{
	uint64 timeStamp = getTimestamp();
	std::string errInfo;

	mysql_conn_Ptr conn;
	MYSQL_GET_CONN_THEN_LOCK(conn);

	std::ostringstream sql;
	sql << "insert into tbl_user (id, name, feat_vec, created_at, updated_at) values("
		<< "'" << user.id << "', "
		<< "'" << user.name << "', "
		<< "'" << feat_vec_to_str(user.feat_vec) << "', "
		<< timeStamp << ", "
		<< timeStamp << ");";

	uint64 last_insert_id = 0;
	uint64 affected_rows = 0;

	conn->autocommit(false);

	int nRet = conn->execute_sql(sql.str(), last_insert_id, affected_rows, errInfo);
	CHECK_RET_ROLL_BACK(nRet, conn);

	sql.str("");
	sql.clear();

	sql << "insert into tbl_img (id, img_path, created_at, updated_at) values("
		<< "'" << user.id << "', "
		<< "'" << img_path << "', "
		<< timeStamp << ", "
		<< timeStamp << ");";

	last_insert_id = 0;
	affected_rows = 0;
	nRet = conn->execute_sql(sql.str(), last_insert_id, affected_rows, errInfo);
	CHECK_RET_ROLL_BACK(nRet, conn);

	conn->commit();
	conn->autocommit(true);

	m_users.push_back(user);
	return 0;
}

int User_Mgt::del_user(user_t &user)
{
	std::string errInfo;

	mysql_conn_Ptr conn;
	MYSQL_GET_CONN_THEN_LOCK(conn);

	std::ostringstream sql;
	sql << "delete from tbl_user where id = '"
		<< user.id << "' and name = '" << user.name << "';";

	uint64 last_insert_id = 0;
	uint64 affected_rows = 0;

	conn->autocommit(false);

	int nRet = conn->execute_sql(sql.str(), last_insert_id, affected_rows, errInfo);
	CHECK_RET_ROLL_BACK(nRet, conn);

	sql.str("");
	sql.clear();

	sql << "delete from tbl_img where id = '"
		<< user.id << "';";

	last_insert_id = 0;
	affected_rows = 0;
	nRet = conn->execute_sql(sql.str(), last_insert_id, affected_rows, errInfo);
	CHECK_RET_ROLL_BACK(nRet, conn);

	conn->commit();
	conn->autocommit(true);

	for (std::vector<user_t>::iterator it = m_users.begin(); it != m_users.end(); ++it)
	{
		if ((*it).id == user.id && (*it).name == user.name)
		{
			m_users.erase(it);
			break;
		}
	}

	dbg("m_users number: %ld\n", m_users.size());
	return 0;
}


