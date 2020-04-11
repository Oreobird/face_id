#ifndef __USER_MGT__
#define __USER_MGT__

#include "base_common.h"
#include "base_singleton_t.h"
#include "base_thread_mutex.h"
#include "base_rw_thread_mutex.h"
#include "base_condition.h"

#include "common.h"


//typedef unsigned long long uint64;
USING_NS_BASE;
using namespace std;

typedef struct _user
{
    std::string id;
    std::string name;
    std::vector<float> feat_vec;
} user_t;


class User_Mgt
{
public:
	User_Mgt();

	~User_Mgt();

	int init();

    std::vector<user_t> *get_users(void);

    int add_user(user_t &user, const std::string img_path);

    int del_user(user_t &user);

    bool user_exist(const std::string &user_id);
    bool user_exist_in_db(const std::string &user_id);

    void dump_feat_vec();

private:
    std::vector<float> str_to_feat_vec(const std::string &feat_vec_str);
    std::string feat_vec_to_str(std::vector<float> &feat_vec);

private:
	Thread_Mutex _mutex;
	std::vector<user_t> m_users;
};

#define PSGT_User_Mgt Singleton_T<User_Mgt>::getInstance()

#endif
