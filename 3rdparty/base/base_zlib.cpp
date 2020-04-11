
#include "base_zlib.h"
#include "zlib.h"


NS_BASE_BEGIN

/*
压缩率(Compression ratio) 文件压缩后的大小与压缩前的大小之比，
比如你把100m的文件压缩后是90m，压缩率就是90/100*100%=90%，
压缩率一般是越小越好，但是压得越小，时间越长。

#define Z_OK            0
#define Z_STREAM_END    1

typedef struct z_stream_s {
    Bytef    *next_in;  --- next input byte
    uInt     avail_in;  --- number of bytes available at next_in
    uLong    total_in;  --- total nb of input bytes read so far

    Bytef    *next_out; --- next output byte should be put there   ----- 输出buffer
    uInt     avail_out; --- remaining free space at next_out           ------  next_out 中多少个字节是非真正输出字节
    uLong    total_out; --- total nb of bytes output so far             ------ 总输出字节数

    char     *msg;      --- last error message, NULL if no error
    struct internal_state FAR *state; --- not visible by applications

    alloc_func zalloc;  --- used to allocate the internal state
    free_func  zfree;   --- used to free the internal state
    voidpf     opaque;  --- private data object passed to zalloc and zfree

    int     data_type;  --- best guess about the data type: binary or text
    uLong   adler;      --- adler32 value of the uncompressed data
    uLong   reserved;   --- reserved for future use
} z_stream;


[例子1]: 说明解压过程输出buffer 大小过小的情况， 例子中定为5字节
# ./test.bin 
zlib version: 1.2.3
---------- Src(len:49) ----------
78 63 70 73 66 64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 66 73 64 74 77 72 71 62 62 
62 62 72 77 62 67 66 67 67 67 67 68 68 68 68 68 78 63 70 
ret of deflate:1
---------- Dst(len:38, avail_out:11, total_out:38) ----------
78 ffffff9c ffffffab 48 2e 28 4e 4b 41 03 69 ffffffc5 29 25 ffffffe5 45 ffffff85 49 40 50 54 ffffff9e 
ffffff94 ffffff9e ffffff96 0e 04 19 20 50 ffffff91 5c 00 00 fffffff2 11 13 ffffffff 


---------- Src(len:38) ----------
78 ffffff9c ffffffab 48 2e 28 4e 4b 41 03 69 ffffffc5 29 25 ffffffe5 45 ffffff85 49 40 50 54 ffffff9e 
ffffff94 ffffff9e ffffff96 0e 04 19 20 50 ffffff91 5c 00 00 fffffff2 11 13 ffffffff 
ret of inflate:0
---------- Dst(len:5, avail_out:0, total_out:5) ----------
78 63 70 73 66 
ret of inflate:0
---------- Dst(len:5, avail_out:0, total_out:10) ----------
64 64 64 64 64 
ret of inflate:0
---------- Dst(len:5, avail_out:0, total_out:15) ----------
64 64 64 64 64 
ret of inflate:0
---------- Dst(len:5, avail_out:0, total_out:20) ----------
64 64 64 64 64 
ret of inflate:0
---------- Dst(len:5, avail_out:0, total_out:25) ----------
64 66 73 64 74 
ret of inflate:0
---------- Dst(len:5, avail_out:0, total_out:30) ----------
77 72 71 62 62 
ret of inflate:0
---------- Dst(len:5, avail_out:0, total_out:35) ----------
62 62 72 77 62 
ret of inflate:0
---------- Dst(len:5, avail_out:0, total_out:40) ----------
67 66 67 67 67 
ret of inflate:0
---------- Dst(len:5, avail_out:0, total_out:45) ----------
67 68 68 68 68 
ret of inflate:1
---------- Dst(len:4, avail_out:1, total_out:49) ----------
68 78 63 70



[例子2]: 50个1压缩后只有12个字节(50/12 = 4.17)，所以不能在解压过程中
简单将输出buffer 尺寸定义为3倍输入buffer尺寸
# ./test.bin 
zlib version: 1.2.3
---------- Src(len:50) ----------
31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 
31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 31 
ret of deflate:1
---------- Dst(len:12, avail_out:38, total_out:12) ----------

*/


int Zlib::zip_mem(char *pSrc, const int src_len, 
					      char* &pDst, int &dst_len, const int level)
{
	int nRet = 0;

	if ((NULL == pSrc) || (src_len < 1))
	{
		printf("(NULL == pSrc) || (src_len < 1)\n");
		return -1;
	}

	pDst = NULL;
	dst_len = 0;

	printf("---------- Src(len:%d) ----------\n", src_len);
	for (int i=0; i<src_len; i++)
	{
		printf("%02x ", pSrc[i]);
	}
	printf("\n");
	

	/*
	为压缩初始化内部流状态，zalloc，zfree和opaque字段必须
	在调用之前初始化，如果zalloc和zfree被初始化为Z_NULL，
	deflateInit会更新它们而使用默认的分配函数。
	*/
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
 
	/*
	压缩级别(-1 ~ 9)
	低压缩级别，压缩速度快但是压缩率高
	高压缩级别，压缩速度慢但是压缩率低
	zlib 中Z_DEFAULT_COMPRESSION=-1， 提供好的压缩速度和压缩率， 等同于6
	压缩级别等于0， 不压缩
	*/
	nRet = deflateInit_(&strm, level, ZLIB_VERSION, sizeof(z_stream));
	if (nRet != Z_OK)
	{
		printf("deflateInit_ failed, error code:%d\n", nRet);
		return nRet;
	}

	//输入buf 的长度和首地址
	strm.avail_in = src_len;
	strm.next_in = (Bytef*)pSrc;

	pDst = (char*)malloc(src_len+1);
	//pDst = new char[src_len+1];
	if(NULL == pDst)
	{
		printf("new failed, byte size:%d, errno:%d, errmsg:%s\n", src_len, errno, strerror(errno));
		return -1;
	}
	
	memset(pDst, 0x0, (src_len+1));
	strm.avail_out = src_len+1;
	strm.next_out = (Bytef*)pDst;

	/*
	  deflate 函数尽可能的压缩数据，
	  当输入缓冲为空或者输出缓冲满的时候, deflate执行结束返回。
	*/
	nRet = deflate(&strm, Z_FINISH);
	if ((Z_STREAM_END == nRet) || (Z_OK == nRet))
	{
		printf("ret of deflate:%d\n", nRet);
		
		nRet = Z_OK;
		
		dst_len = src_len + 1 - strm.avail_out;

		printf("---------- Dst(len:%d, avail_out:%u, total_out:%lu) ----------\n", 
			dst_len, strm.avail_out, strm.total_out);
		
		for (int i=0; i<dst_len; i++)
		{
			printf("%02x ", pDst[i]);
		}
		printf("\n");
	}
	else
	{
		printf("deflate failed, error code:%d", nRet);
		if(strm.msg)
		{
			printf(", error msg:%s", strm.msg);
		}
		printf("\n");

		DELETE_POINTER_ARR(pDst);
	}

	/* clean up and return */
	deflateEnd(&strm);
	
	return nRet;
	
}





int Zlib::unzip_mem(char *pSrc, const int src_len, 
						     char* &pDst, int &dst_len)
{
	int nRet = 0;

	if ((NULL == pSrc) || (src_len < 1))
	{
		printf("(NULL == pSrc) || (src_len < 1)\n");
		return -1;
	}

	pDst = NULL;
	dst_len = 0;

	printf("---------- Src(len:%d) ----------\n", src_len);
	for (int i=0; i<src_len; i++)
	{
		printf("%02x ", pSrc[i]);
	}
	printf("\n");
	

	/* allocate inflate state */
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	nRet = inflateInit_(&strm, ZLIB_VERSION, sizeof(z_stream));
	if (nRet != Z_OK)
	{
		printf("deflateInit_ failed, error code:%d\n", nRet);
		return nRet;
	}

	strm.avail_in = src_len;
	strm.next_in = (Bytef*)pSrc;

	char *pDstNew = NULL;
	int multiple = 1;
	do 
	{
		/*
		缺省输出buffer的长度为输入长度的3倍，但是可能这个空间
		在解压过程还是不够的这个时候就要进行相应的处理
		*/
		char *pTmp = (char*)malloc(3*src_len);
		//char *pTmp = new char[3*src_len];
		if(NULL == pTmp)
		{
			printf("new pTmp failed, byte size:3*%d, errno:%d, errmsg:%s\n", 
				src_len, errno, strerror(errno));
			return -1;
		}		
		memset(pTmp, 0x0, (3*src_len)); 
		
		strm.avail_out = 3*src_len;
		strm.next_out = (Bytef*)pTmp;

		nRet = inflate(&strm, Z_NO_FLUSH);
		if ((Z_STREAM_END == nRet) || (Z_OK == nRet))
		{
			printf("ret of inflate:%d\n", nRet);

			if(Z_STREAM_END == nRet)
			{
				nRet = Z_OK;
			}

			dst_len = 3*src_len - strm.avail_out;

			printf("---------- Dst(len:%d, avail_out:%u, total_out:%lu) ----------\n", 
				dst_len, strm.avail_out, strm.total_out);
			
			for (int i=0; i<dst_len; i++)
			{
				printf("%02x ", pTmp[i]);
			}
			printf("\n");

			/*
			void *realloc(void *ptr, size_t size);
			
			使用realloc 注意点
			1. realloc失败的时候，返回NULL
			2. realloc失败的时候，原来的内存不改变，也就是不free或不move，(这个地方很容易出错)
			3. 假如原来的内存后面还有足够多剩余内存的话，
			        realloc的内存=原来的内存+剩余内存,realloc还是返回原来内存的地址; 
			   假如原来的内存后面没有足够多剩余内存的话，
			       realloc将申请新的内存，然后把原来的内存数据拷贝到新内存里，
			   原来的内存将被free掉,realloc返回新内存的地址
			4. 如果size为0，效果等同于free()，返回NULL
			5. 传递给realloc的指针必须是先前通过malloc(), calloc(), 或realloc()分配的
			6. 传递给realloc的指针可以为空，等同于malloc
			7. 如果size 小于等于原来的大小，返回原来的地址
			*/

			//定义了一个临时变量pNewTmp， 防止realloc失败后把pDstNew 赋值为NULL
			char *pNewTmp = (char*)realloc(pDstNew, (multiple*3*src_len));
			if(NULL == pNewTmp)
			{	
				printf("realloc pDstNew failed, byte size:%d*3*%d, errno:%d, errmsg:%s\n", 
					multiple, src_len, errno, strerror(errno));

				DELETE_POINTER_ARR(pTmp);
				DELETE_POINTER(pDstNew);
				return -1;
			}
			
			pDstNew = pNewTmp;
			memcpy((pDstNew+((multiple-1)*3*src_len)), pTmp, (3*src_len));
			
			multiple++;

			DELETE_POINTER_ARR(pTmp);
		
		}
		else
		{
			printf("inflate failed, error code:%d", nRet);
			if(strm.msg)
			{
				printf(", error msg:%s", strm.msg);
			}
			printf("\n");
		
			break;
		}

	} while (strm.avail_out == 0);
	

	dst_len = strm.total_out;
	pDst = pDstNew;
		
	/* clean up and return */
	inflateEnd(&strm);

	return nRet;
	
}




int Zlib::gzip_mem(char *pSrc, const int src_len, 
					      char* &pDst, int &dst_len, const int windowBits, const int memLevel, const int level)
{
	int nRet = 0;

	if ((NULL == pSrc) || (src_len < 1))
	{
		printf("(NULL == pSrc) || (src_len < 1)\n");
		return -1;
	}

	pDst = NULL;
	dst_len = 0;

	printf("---------- Src(len:%d) ----------\n", src_len);
	for (int i=0; i<src_len; i++)
	{
		printf("%02x ", pSrc[i]);
	}
	printf("\n");
	

	/*
	为压缩初始化内部流状态，zalloc，zfree和opaque字段必须
	在调用之前初始化，如果zalloc和zfree被初始化为Z_NULL，
	deflateInit会更新它们而使用默认的分配函数。
	*/
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
 
	/*
	压缩级别(-1 ~ 9)
	低压缩级别，压缩速度快但是压缩率高
	高压缩级别，压缩速度慢但是压缩率低
	zlib 中Z_DEFAULT_COMPRESSION=-1， 提供好的压缩速度和压缩率， 等同于6
	压缩级别等于0， 不压缩
	*/
	nRet = deflateInit2_(&strm, level, Z_DEFLATED, windowBits, memLevel, Z_DEFAULT_STRATEGY, ZLIB_VERSION, sizeof(z_stream));
	if (nRet != Z_OK)
	{
		printf("deflateInit2_ failed, error code:%d\n", nRet);
		return nRet;
	}

	//输入buf 的长度和首地址
	strm.avail_in = src_len;
	strm.next_in = (Bytef*)pSrc;

	pDst = (char*)malloc(src_len+1);
	//pDst = new char[src_len+1];
	if(NULL == pDst)
	{
		printf("new failed, byte size:%d, errno:%d, errmsg:%s\n", src_len, errno, strerror(errno));
		return -1;
	}
	
	memset(pDst, 0x0, (src_len+1));
	strm.avail_out = src_len+1;
	strm.next_out = (Bytef*)pDst;

	/*
	  deflate 函数尽可能的压缩数据，
	  当输入缓冲为空或者输出缓冲满的时候, deflate执行结束返回。
	*/
	nRet = deflate(&strm, Z_FINISH);
	if ((Z_STREAM_END == nRet) || (Z_OK == nRet))
	{
		printf("ret of deflate:%d\n", nRet);
		
		nRet = Z_OK;
		
		dst_len = src_len + 1 - strm.avail_out;

		printf("---------- Dst(len:%d, avail_out:%u, total_out:%lu) ----------\n", 
			dst_len, strm.avail_out, strm.total_out);
		
		for (int i=0; i<dst_len; i++)
		{
			printf("%02x ", pDst[i]);
		}
		printf("\n");
	}
	else
	{
		printf("deflate failed, error code:%d", nRet);
		if(strm.msg)
		{
			printf(", error msg:%s", strm.msg);
		}
		printf("\n");

		DELETE_POINTER_ARR(pDst);
	}

	/* clean up and return */
	deflateEnd(&strm);
	
	return nRet;
	
}




std::string Zlib::version()
{
	std::string version = zlibVersion();
	return version;
}


NS_BASE_END


