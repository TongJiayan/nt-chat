//mysql_demo.cpp

#include "db.h"
#include <stdint.h>
#include <stdlib.h>

//初始化数据库
enum RETURNTYPE InitDatabase(MYSQL **_mysql)
{
    /* 初始化 mysql 变量，失败返回NULL */
    if ((*_mysql = mysql_init(NULL)) == NULL)
    {
        printf("mysql_init failed\n");
        return FALSE;
    }

    /* 连接数据库，失败返回NULL
       1、mysqld没运行
       2、没有指定名称的数据库存在 */
    if (mysql_real_connect(*_mysql, "localhost", "root", "123456", "db1551445", 0, NULL, 0) == NULL)
    {
        printf("mysql_real_connect failed(%s)", mysql_error(*_mysql));
        return FALSE;
    }

    /* 设置字符集，否则读出的字符乱码，即使/etc/my.cnf中设置也不行 */
    mysql_set_character_set(*_mysql, "gbk");

    return TRUE;
}

//判断该用户是不是首次登陆
enum RETURNTYPE JudgeFirstLog(MYSQL *_mysql, char *_username)
{
    MYSQL_RES *result;
    MYSQL_ROW row;

    /* 进行查询，成功返回0，不成功非0
    1、查询字符串存在语法错误
    2、查询不存在的数据表 */
    char sql[200];
    sprintf(
        sql,
        "select firstlog from user where username='%s'",
        _username);

    if (mysql_query(_mysql, sql))
    {
        printf("mysql_query_connect failed(%s)", mysql_error(_mysql));
        return FALSE;
    }

    /* 将查询结果存储起来，出现错误则返回NULL
       注意：查询结果为NULL，不会返回NULL */
    if ((result = mysql_store_result(_mysql)) == NULL)
    {
        printf("mysql_store_result failed");
        return FALSE;
    }

    /* 循环读取所有满足条件的记录
       1、返回的列顺序与select指定的列顺序相同，从row[0]开始
       2、不论数据库中是什么类型，C中都当作是字符串来进行处理，如果有必要，需要自己进行转换
       3、根据自己的需要组织输出格式 */
    int resultnum = 0;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        resultnum = atoi(row[0]);
    }

    if (resultnum == 1)
    {
        //首次登陆 - 将数据库写
        char tmpsql[200];
        sprintf(
            tmpsql,
            "update user set firstlog=false where username='%s';",
            _username);

        if (mysql_query(_mysql, tmpsql))
        {
            printf("mysql_query_connect failed(%s)", mysql_error(_mysql));
            return FALSE;
        }

        /* 释放result */
        mysql_free_result(result);

        return TRUE;
    }
    else
    {
        /* 释放result */
        mysql_free_result(result);
        return FALSE;
    }
}

//判断用户密码是否正确 1-正确 2-密码错误 3-用户名不存在 4-首次登陆需要改密[前提：用户名与密码正确]
// _keyword是明文密码
int JudgeUser(MYSQL *_mysql, char *_username, char *_keyword)
{
    MYSQL_RES *result;
    MYSQL_ROW row;

	/* 为明文密码加密 */
	char *enKeyword;
	encrypt(_keyword,&enKeyword);
	
    /* 进行查询，成功返回0，不成功非0
    1、查询字符串存在语法错误
    2、查询不存在的数据表 */
    char sql[200];
    sprintf(
        sql,
        "select count(username) from user where username='%s' and keyword='%s'",
        _username,
        enKeyword);

    if (mysql_query(_mysql, sql))
    {
        printf("mysql_query_connect failed(%s)", mysql_error(_mysql));
        return FALSE;
    }

    /* 将查询结果存储起来，出现错误则返回NULL
       注意：查询结果为NULL，不会返回NULL */
    if ((result = mysql_store_result(_mysql)) == NULL)
    {
        printf("mysql_store_result failed");
        return FALSE;
    }

    /* 打印当前查询到的记录的数量 */
    //printf("select return %d records\n",mysql_num_rows(result));

    /* 循环读取所有满足条件的记录
       1、返回的列顺序与select指定的列顺序相同，从row[0]开始
       2、不论数据库中是什么类型，C中都当作是字符串来进行处理，如果有必要，需要自己进行转换
       3、根据自己的需要组织输出格式 */
    int resultnum = 0;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        resultnum = atoi(row[0]);
    }

    if (resultnum == 1)
    {
        /* 用户名和密码正确，判断是不是首次登陆，需不需要改密 */
        if (JudgeFirstLog(_mysql, _username) == TRUE)
        {
            /* 释放result */
            mysql_free_result(result);

            //首次登陆，需要改密
            return 4;
        }
        else
        {
            /* 释放result */
            mysql_free_result(result);

            return 1;
        }
    }
    else
    {
        /* 用户名和密码其中一方错误 - 判断是哪个错了*/
        sprintf(
            sql,
            "select count(username) from user where username='%s'",
            _username);

        if (mysql_query(_mysql, sql))
        {
            printf("mysql_query_connect failed(%s)", mysql_error(_mysql));
            return -1;
        }

        /* 将查询结果存储起来，出现错误则返回NULL
       注意：查询结果为NULL，不会返回NULL */
        if ((result = mysql_store_result(_mysql)) == NULL)
        {
            printf("mysql_store_result failed");
            return -1;
        }

        /* 打印当前查询到的记录的数量 */
        //printf("select return %d records\n",mysql_num_rows(result));

        /* 循环读取所有满足条件的记录
       1、返回的列顺序与select指定的列顺序相同，从row[0]开始
       2、不论数据库中是什么类型，C中都当作是字符串来进行处理，如果有必要，需要自己进行转换
       3、根据自己的需要组织输出格式 */
        resultnum = 0;
        while ((row = mysql_fetch_row(result)) != NULL)
        {
            resultnum = atoi(row[0]);
        }

        if (resultnum == 1)
        {
            //用户名是存在的 - 密码出错了
            /* 释放result */
            mysql_free_result(result);
            return 2;
        }
        else
        {
            //用户名不存在
            mysql_free_result(result);
            return 3;
        }
    }
}

//更新数据库username的密码为_keywork
enum RETURNTYPE UpdateSecret(MYSQL *_mysql, char *_username, char *_keyword)
{
	/* 为明文密码加密 */
	char *enKeyword;
	encrypt(_keyword,&enKeyword);
	
    char sql[200];
    sprintf(
        sql,
        "update user set keyword='%s' where username='%s'; ",
        enKeyword,
        _username);

    if (mysql_query(_mysql, sql))
    {
        printf("mysql_query_connect failed(%s)", mysql_error(_mysql));
        return FALSE;
    }

    return TRUE;
}

//当前在线用户增加用户
enum RETURNTYPE AddOnlineUser(MYSQL *_mysql, char *_username, int id)
{
    /* 进行查询，成功返回0，不成功非0
    1、查询字符串存在语法错误
    2、查询不存在的数据表 */
    char sql[200];
    sprintf(
        sql,
        "insert into onlineuser(username,id) values ('%s',%d); ",
        _username,
        id);

    if (mysql_query(_mysql, sql))
    {
        printf("mysql_query_connect failed(%s)", mysql_error(_mysql));
        return FALSE;
    }

    return TRUE;
}

//当前在线用户减少用户
enum RETURNTYPE DelOnlineUser(MYSQL *_mysql, char *_username)
{
    /* 进行查询，成功返回0，不成功非0
    1、查询字符串存在语法错误
    2、查询不存在的数据表 */
    char sql[200];
    sprintf(
        sql,
        "delete from onlineuser where username='%s'; ",
        _username);

    if (mysql_query(_mysql, sql))
    {
        printf("mysql_query_connect failed(%s)", mysql_error(_mysql));
        return FALSE;
    }

    return TRUE;
}

//返回当前在线用户的文件名
char *GetOnlineUsername(MYSQL *_mysql, int _id)
{
    MYSQL_RES *result;
    MYSQL_ROW row;

    /* 进行查询，成功返回0，不成功非0
    1、查询字符串存在语法错误
    2、查询不存在的数据表 */
    char sql[200];
    sprintf(
        sql,
        "select username from onlineuser where id=%d;",
        _id);

    if (mysql_query(_mysql, sql))
    {
        printf("mysql_query_connect failed(%s)", mysql_error(_mysql));
        return NULL;
    }

    /* 将查询结果存储起来，出现错误则返回NULL
       注意：查询结果为NULL，不会返回NULL */
    if ((result = mysql_store_result(_mysql)) == NULL)
    {
        printf("mysql_store_result failed");
        return NULL;
    }

    /* 循环读取所有满足条件的记录
       1、返回的列顺序与select指定的列顺序相同，从row[0]开始
       2、不论数据库中是什么类型，C中都当作是字符串来进行处理，如果有必要，需要自己进行转换
       3、根据自己的需要组织输出格式 */
    char *buf = (char *)malloc(16 * sizeof(char));
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        sprintf(buf, "%s", row[0]);
    }

    /* 释放result */
    mysql_free_result(result);

    return buf;
}

//返回当前在线用户的ID号
int GetOnlineId(MYSQL *_mysql, char *_username)
{
    MYSQL_RES *result;
    MYSQL_ROW row;

    /* 进行查询，成功返回0，不成功非0
    1、查询字符串存在语法错误
    2、查询不存在的数据表 */
    char sql[200];
    sprintf(
        sql,
        "select id from onlineuser where username='%s';",
        _username);

    if (mysql_query(_mysql, sql))
    {
        printf("mysql_query_connect failed(%s)", mysql_error(_mysql));
        return -1;
    }

    /* 将查询结果存储起来，出现错误则返回NULL
       注意：查询结果为NULL，不会返回NULL */
    if ((result = mysql_store_result(_mysql)) == NULL)
    {
        printf("mysql_store_result failed");
        return -1;
    }

    /* 循环读取所有满足条件的记录
       1、返回的列顺序与select指定的列顺序相同，从row[0]开始
       2、不论数据库中是什么类型，C中都当作是字符串来进行处理，如果有必要，需要自己进行转换
       3、根据自己的需要组织输出格式 */
    int buf;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        buf = atoi(row[0]);
    }

    /* 释放result */
    mysql_free_result(result);

    return buf;
}

//包装发到数据库的msg
void PackMsg(char _msg[], int _len)
{
    int i = 0;
    for (i = 0; i < _len; i++)
        if ((_msg)[i] == 0)
            (_msg)[i] = '`';
}

void UnpackMsg(char *_msg, int _len)
{
    int i = 0;
    for (i = 0; i < _len; i++)
        if ((_msg)[i] == '`')
            (_msg)[i] = 0;
}

/***************************************************************************
 * 函数名称：GetAllUsers
 * 功    能：得到所有的在线用户
 * 输入参数：
 * 返 回 值：char*
 * 说    明：如果当前数据库中在线用户有：MiaoMiaoYang tongjiayan
 *          返回值为 @tongjiayan@MiaoMiaoYang#
 *          每一个用户以@开头，结束时以#结尾 
 ***************************************************************************/
char *GetAllOnlineUsers(MYSQL *_mysql)
{
    MYSQL_RES *result;
    MYSQL_ROW row;

    /* 进行查询，成功返回0，不成功非0
    1、查询字符串存在语法错误
    2、查询不存在的数据表 */
    char sql[200];
    sprintf(
        sql,
        "select * from onlineuser;");

    if (mysql_query(_mysql, sql))
    {
        printf("mysql_query_connect failed(%s)", mysql_error(_mysql));
        return NULL;
    }

    /* 将查询结果存储起来，出现错误则返回NULL
       注意：查询结果为NULL，不会返回NULL */
    if ((result = mysql_store_result(_mysql)) == NULL)
    {
        printf("mysql_store_result failed");
        return NULL;
    }

    /* 循环读取所有满足条件的记录
       1、返回的列顺序与select指定的列顺序相同，从row[0]开始
       2、不论数据库中是什么类型，C中都当作是字符串来进行处理，如果有必要，需要自己进行转换
       3、根据自己的需要组织输出格式 */
    char *buf = NULL;
    int size = 0;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        if (buf == NULL)
        {
            size = strlen(row[0]) + 2; //@ + row[0]字符串的长度(不包括\0) + \0
            buf = (char *)malloc(size * sizeof(char));
            sprintf(buf, "@%s", row[0]);
        }
        else
        {
            char *tmpbuf = NULL;
            size += strlen(row[0]) + 1; //之前的字符串(包含\0) + @ + row[0]字符串长度
            tmpbuf = (char *)malloc(size * sizeof(char));
            sprintf(tmpbuf, "%s@%s", buf, row[0]);
            free(buf);
            buf = tmpbuf;
        }
    }

    size++; //size 多加一个#
    char *final = (char *)malloc(sizeof(char) * (size));
    sprintf(final, "%s#", buf);
    free(buf);

    /* 释放result */
    mysql_free_result(result);

    return final;
}

/***************************************************************************
 * 函数名称：GetAllUsers
 * 功    能：得到所有的用户
 * 输入参数：
 * 返 回 值：char*
 * 说    明：如果当前数据库中用户有：MiaoMiaoYang tongjiayan zhengxuanci
 *          返回值为 @MiaoMiaoYang1@tongjiayan1@zhengxuanci0#
 *          每一个用户以@开头，最后的数字为1表示当前在线，为0表示当前不在线，结束时以#结尾 
 ***************************************************************************/
char *GetAllUsers(MYSQL *_mysql)
{
    MYSQL_RES *result;
    MYSQL_ROW row;

    /* 进行查询，成功返回0，不成功非0
    1、查询字符串存在语法错误
    2、查询不存在的数据表 */
    char sql[200];
    sprintf(
        sql,
        "select user.username, (select count(onlineuser.username) from onlineuser where onlineuser.username=user.username) count from user;");
    /***********************************
     * 返回的结果说明：
     * username:
     * @MiaoMiaoYang @zhengxuanci @tongjiayan
     * 
     * onlineuser:
     * @MiaoMiaoYang
     * 
     * 查询返回结果：
     * +--------------+-------+
     * | username     | count |
     * +--------------+-------+ 
     * | MiaoMiaoYang |     1 |
     * | tongjiayan   |     0 |
     * | zhengxuanci  |     0 |
     * +--------------+-------+
     **********************************/

    if (mysql_query(_mysql, sql))
    {
        printf("mysql_query_connect failed(%s)", mysql_error(_mysql));
        return NULL;
    }

    /* 将查询结果存储起来，出现错误则返回NULL
       注意：查询结果为NULL，不会返回NULL */
    if ((result = mysql_store_result(_mysql)) == NULL)
    {
        printf("mysql_store_result failed");
        return NULL;
    }

    /* 循环读取所有满足条件的记录
       1、返回的列顺序与select指定的列顺序相同，从row[0]开始
       2、不论数据库中是什么类型，C中都当作是字符串来进行处理，如果有必要，需要自己进行转换
       3、根据自己的需要组织输出格式 */
    char *buf = NULL;
    int size = 0;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        if (buf == NULL)
        {
            size = strlen(row[0]) + 3;
            buf = (char *)malloc(size * sizeof(char));
            sprintf(buf, "@%s%s", row[0], row[1]);
        }
        else
        {
            char *tmpbuf = NULL;
            size += strlen(row[0]) + 3;
            tmpbuf = (char *)malloc(size * sizeof(char));
            sprintf(tmpbuf, "%s@%s%s", buf, row[0], row[1]);
            free(buf);
            buf = tmpbuf;
        }
    }

    char *final = (char *)malloc(sizeof(char) * (size + 1));
    sprintf(final, "%s#", buf);
    free(buf);

    /* 释放result */
    mysql_free_result(result);

    return final;
}

//得到每一条服务端子进程向客户端所发的消息
int GetSendMessage(MYSQL *_mysql, char *_username, char ***_return)
{
    MYSQL_RES *result;
    MYSQL_ROW row;

    /* 进行查询，成功返回0，不成功非0
    1、查询字符串存在语法错误
    2、查询不存在的数据表 */
    char sql[200];
    sprintf(
        sql,
        "select msg from cMessage where to_user='%s' and read_flag=true;",
        _username);

    if (mysql_query(_mysql, sql))
    {
        printf("mysql_query_connect failed(%s)", mysql_error(_mysql));
        return -1;
    }

    /* 将查询结果存储起来，出现错误则返回NULL
       注意：查询结果为NULL，不会返回NULL */
    if ((result = mysql_store_result(_mysql)) == NULL)
    {
        printf("mysql_store_result failed");
        return -1;
    }

    /* 获得数据库中所查询到的数据的数量 - 建立空间 */
    int resultnum = (int)mysql_num_rows(result);
    (*_return) = (char **)malloc(sizeof(char *) * resultnum); //三级指针申请二维数组的空间
    int i = 0;
    for (i = 0; i < resultnum; i++)
    {
        (*_return)[i] = (char *)malloc(sizeof(char) * 1024);
        //二级指针申请一维数组的空间，每个一维数组的空间为1024字节 - 对应数据库max 1024
    }

    /* 循环读取所有满足条件的记录
       1、返回的列顺序与select指定的列顺序相同，从row[0]开始
       2、不论数据库中是什么类型，C中都当作是字符串来进行处理，如果有必要，需要自己进行转换
       3、根据自己的需要组织输出格式 */

    //printf("\n ----------------- \n 数据库代码中测试 \n");

    i = 0;
    while ((row = mysql_fetch_row(result)) != NULL && i < resultnum)
    {

        //首先判断一下帧的大小
        uint16_t len = 0;
        char tmp_buf[2] = {row[0][2], row[0][3]};
        UnpackMsg(tmp_buf, 2);
        memcpy(&len, tmp_buf, 2 * sizeof(char));

        // memcpy(&len, &(row[0][3]), 2);
        // Str2int2(row[0], len);

        memcpy((*_return)[i], row[0], len * sizeof(char));

        UnpackMsg((*_return)[i], len);

        // Str2int2((*_return)[i], len);

        i++;

        /* * * * *
     *向数据库更新已经完成的这些操作     * 
     * * * * */

        char update_sql[200];
        sprintf(
            update_sql,
            "update cMessage set read_flag=false where to_user='%s' and msg='%s' and read_flag=true",
            _username,
            row[0]);

        if (mysql_query(_mysql, update_sql))
        {
            printf("mysql_query_connect failed(%s)", mysql_error(_mysql));
            return -1;
        }
    }

    /* 释放result */
    mysql_free_result(result);

    return resultnum;
}

//服务端子进程向其他子进程发送包装好的帧
void SetMessageToDB(MYSQL *_mysql, char *fromuser, char *touser, char *_msg)
{
    /* 进行查询，成功返回0，不成功非0
    1、查询字符串存在语法错误
    2、查询不存在的数据表 */

    uint16_t len=0;
    memcpy(&len,&(_msg[2]),2);

    char* msg=(char*)malloc(sizeof(char)*(len+1));
    memcpy(msg,_msg,len);

    PackMsg(msg,len);
    msg[len]='\0';


    //生产查询语句的前半部分
    char sql[200];
    sprintf(
        sql,
        "insert into cMessage(from_user,to_user,msg) values ('%s','%s','%s')",
        fromuser,
        touser,
        msg);

    if (mysql_query(_mysql, sql))
    {
        printf("mysql_query_connect failed(%s)", mysql_error(_mysql));
        return;
    }
};

//密码加密存储
void encrypt(char source[],char **target)
{
	unsigned int length=strlen(source);
	*target=(char *)malloc((length+1) * sizeof(char));
	strcpy(*target,source);
	if(NULL==*target)
	{
		printf("Alloc fail!\n");
		return;
	}
	for(int i=0;i<length-1;i+=2)
	{
		(*target)[i]=source[i+1];
		(*target)[i+1]=source[i];
	}
	return ;
}



//#define TESTDB
#ifdef TESTDB

#include <unistd.h>
#include <stdio.h>

void Str2int2(char *buf, int length)
{
    int i;
    for (i = 0; i < length; i++)
    {
        if ((buf[i] >= 'a' && buf[i] <= 'z') || (buf[i] >= 'A' && buf[i] <= 'Z') || (buf[i] >= '0' && buf[i] <= '9') || (buf[i] == '@'))
            printf("%c ", buf[i]);
        else
            printf("%x ", (int)buf[i]);
        // if(i!=0&&i%4==0)
        //     printf(" ");
    }
    printf("%c", '\n');
}

//测试多个子进程能不能同时对数据库读写
void test1()
{
    int i = 0;
    for (i = 0; i < 10; i++)
    {
        pid_t fpid = fork();
        if (fpid == 0)
        {
            MYSQL *mysql;

            InitDatabase(&mysql);

            char username1[16] = "MiaoMiaoYang";
            sprintf(username1, "%s%d", username1, i);
            AddOnlineUser(mysql, username1, i);

            //子进程
            while (1)
            {
                char *buf = GetAllOnlineUsers(mysql);
                if (buf != NULL)
                {
                    printf("fork:%d - %s\n", i, buf);
                    free(buf);
                }
                else
                {
                    printf("empty\n");
                }

                sleep(1);
            }

            mysql_close(mysql);
        }
        else if (fpid < 0)
        {
            printf("%d fork()失败", i);
        }
    }

    printf("父进程结束");
    while (1)
    {
        sleep(1);
    }
}

void test2()
{

    MYSQL *mysql;

    InitDatabase(&mysql);

    char username1[16] = "MiaoMiaoYang";
    char username2[16] = "MiaoMiaoYan";
    char username3[16] = "MiaoMiaoYag";
    char username4[16] = "MiaoMiaoYng";
    char username5[16] = "MiaoMiaoang";
    char keyword1[12] = "123456";
    char keyword2[12] = "mkamskksa";

    UpdateSecret(mysql, username1, keyword2);

    printf("首次登陆-4:%d\n", JudgeUser(mysql, username1, keyword1));
    printf("密码错误-2:%d\n", JudgeUser(mysql, username1, keyword2));
    printf("用户消失-3:%d\n", JudgeUser(mysql, username2, keyword1));
    printf("正    确-1:%d\n", JudgeUser(mysql, username1, keyword1));

    UpdateSecret(mysql, username1, keyword2);

    AddOnlineUser(mysql, username1, 1);
    AddOnlineUser(mysql, username2, 2);
    AddOnlineUser(mysql, username3, 3);
    AddOnlineUser(mysql, username4, 4);
    AddOnlineUser(mysql, username5, 5);

    printf("%d\n", GetOnlineId(mysql, username1));
    printf("%s\n", GetOnlineUsername(mysql, 2));

    char *buf = GetAllUsers(mysql);
    if (buf != NULL)
    {
        printf("%s\n", buf);
        free(buf);
    }
    else
    {
        printf("empty\n");
    }

    DelOnlineUser(mysql, username1);
    DelOnlineUser(mysql, username2);
    DelOnlineUser(mysql, username3);
    DelOnlineUser(mysql, username4);
    DelOnlineUser(mysql, username5);

    buf = GetAllUsers(mysql);
    if (buf != NULL)
    {
        printf("%s\n", buf);
        free(buf);
    }

    char from[] = "MiaoMiaoYang";
    char to[] = "zhengxuanci";

    char msg1[20] = "0000hhhhhhh1";
    char msg2[20] = "0000hhhhhhh2";
    char msg3[20] = "0000hhhhhhh3";
    char msg4[20] = "0000hhhhhhh4";
    char msg5[20] = "0000hhhhhhh5";

    uint16_t num = 12;
    memcpy(&(msg1[2]), &num, 2);
    memcpy(&(msg2[2]), &num, 2);
    memcpy(&(msg3[2]), &num, 2);
    memcpy(&(msg4[2]), &num, 2);
    memcpy(&(msg5[2]), &num, 2);

    PackMsg(msg1, num);
    PackMsg(msg2, num);
    PackMsg(msg3, num);
    PackMsg(msg4, num);
    PackMsg(msg5, num);

    SetMessageToDB(mysql, from, to, msg1);
    SetMessageToDB(mysql, from, to, msg2);
    SetMessageToDB(mysql, from, to, msg3);
    SetMessageToDB(mysql, from, to, msg4);
    SetMessageToDB(mysql, from, to, msg5);

    printf("\n-------------------------\n第一次查询\n");

    char **get = NULL;
    int gnum = GetSendMessage(mysql, to, &get);
    printf("num:%d\n", gnum);

    int i = 0;
    for (i = 0; i < gnum; i++)
    {
        Str2int2(get[i], num);
    }

    printf("\n-------------------------\n第2次查询\n");
    get = NULL;

    gnum = GetSendMessage(mysql, to, &get);
    printf("num:%d\n", gnum);

    for (i = 0; i < gnum; i++)
    {
        Str2int2(get[i], num);
    }

    /* 关闭整个连接 */
    mysql_close(mysql);
}

void test3()
{
    MYSQL *mysql;

    InitDatabase(&mysql);

    char *buf = NULL;

    buf = GetAllOnlineUsers(mysql);
    printf("%s\n", buf);

    /* 关闭整个连接 */
    mysql_close(mysql);
}

void getcmessage()
{
    MYSQL *_mysql;

    InitDatabase(&_mysql);

    MYSQL_RES *result;
    MYSQL_ROW row;

    /* 进行查询，成功返回0，不成功非0
    1、查询字符串存在语法错误
    2、查询不存在的数据表 */
    char sql[200];
    sprintf(
        sql,
        "select * from cMessage;");

    if (mysql_query(_mysql, sql))
    {
        printf("mysql_query_connect failed(%s)", mysql_error(_mysql));
        return;
    }

    /* 将查询结果存储起来，出现错误则返回NULL
       注意：查询结果为NULL，不会返回NULL */
    if ((result = mysql_store_result(_mysql)) == NULL)
    {
        printf("mysql_store_result failed");
        return;
    }

    /* 获得数据库中所查询到的数据的数量 - 建立空间 */
    int resultnum = (int)mysql_num_rows(result);
    int i = 0;

    /* 循环读取所有满足条件的记录
       1、返回的列顺序与select指定的列顺序相同，从row[0]开始
       2、不论数据库中是什么类型，C中都当作是字符串来进行处理，如果有必要，需要自己进行转换
       3、根据自己的需要组织输出格式 */

    //printf("\n ----------------- \n 数据库代码中测试 \n");

    i = 0;
    while ((row = mysql_fetch_row(result)) != NULL && i < resultnum)
    {
        printf("from_user: %s to_user: %s cmessage:\n",row[0],row[1]);
        Str2int2(row[2],20);
        printf("\n");
    }

    /* 释放result */
    mysql_free_result(result);

    /* 关闭整个连接 */
    mysql_close(_mysql);
}

int main(int argc, char *argv[])
{
    getcmessage();
    return 0;
}

#endif

