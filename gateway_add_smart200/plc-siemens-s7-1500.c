#include "plc-siemens-s7-1500.h"
#include "typedef.h"
#include "msgQueue.h"
#include "plcMain.h"

PyObject *client;
PyObject *pmodule;
//PyObject *pmodule,*pfunc,*pArg,*pResult;

Flag plc_connect_flag = disconnected;

PyObject *create_plc_client()
{
	PyObject *pfunc = PyObject_GetAttrString(pmodule, "create_plc_client");
	PyObject *pResult = PyEval_CallObject(pfunc,NULL);

	return pResult;
}

void destroy_plc_client(PyObject *client)
{
	PyObject *pfunc = PyObject_GetAttrString(pmodule, "destroy_plc_client");
	PyEval_CallObject(pfunc,client);
}

void disconnect_plc(PyObject *client)
{
	PyObject *pfunc = PyObject_GetAttrString(pmodule, "plc_con_close");
	PyEval_CallObject(pfunc,client);
}

void SIEMENS_S7_1500_Init()
{
	Py_Initialize();

	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append('./')");
	pmodule = PyImport_ImportModule("s7-1500");
}


int connect_plc(PyObject *client,char *ip,long rack,long slot)//0,1
{
	PyObject *pfunc = PyObject_GetAttrString(pmodule, "plc_connect");

//	PyObject *pArg = PyTuple_New(4);//new and free
//	PyTuple_SetItem(pArg,0,client);
//	PyTuple_SetItem(pArg,1,PyString_FromString(ip));
//	PyTuple_SetItem(pArg,2,PyInt_FromLong(rack));
//	PyTuple_SetItem(pArg,3,PyInt_FromLong(slot));

	PyObject *pArg = Py_BuildValue("(Osll)",client,ip,rack,slot);

	PyObject *pResult = PyEval_CallObject(pfunc,pArg);
	int ret = -1;
	PyArg_Parse(pResult, "i", &ret);
	return ret;
}

int read_plc(PyObject *client,char *addr,int *value)
{
	PyObject *pfunc = PyObject_GetAttrString(pmodule, "read");

//	PyObject *pArg = PyTuple_New(2);
//	PyTuple_SetItem(pArg,0,client);
//	PyTuple_SetItem(pArg,1,PyString_FromString(addr));

	PyObject *pArg = Py_BuildValue("(Os)",client,addr);

	PyObject *pResult = PyEval_CallObject(pfunc,pArg);
	int result = -1;
	int v = 0;
	PyArg_ParseTuple(pResult, "ii", &result,&v);
	if(result == 0)
	{
		*value = v;
	}
	else
	{
		*value = 0;
	}
	return result;
}

int readDB_plc(PyObject *client,char *db_num,char *offset,
		char *dataType,int dataSize,void *data)
{
	PyObject *pfunc = PyObject_GetAttrString(pmodule, "DBRead");

//	PyObject *pArg = PyTuple_New(4);
//	PyTuple_SetItem(pArg,0,client);
//	PyTuple_SetItem(pArg,1,PyString_FromString(db_num));
//	PyTuple_SetItem(pArg,2,PyString_FromString(dataType));
//	PyTuple_SetItem(pArg,3,PyString_FromString(offset));

	PyObject *pArg = Py_BuildValue("(Osss)",client,db_num,dataType,offset);

	PyObject *pResult = PyEval_CallObject(pfunc,pArg);
	int result = -1;
	PyObject *py_data = NULL;
	PyArg_ParseTuple(pResult, "iO", &result,&py_data);
	if(result == -1)
	{
		puts("DBRead err.");
		return -1;
	}
	if(strcmp(dataType,"Bool")==0 || strcmp(dataType,"Byte")==0 ||
			strcmp(dataType,"Int")==0 || strcmp(dataType,"DInt")==0 || strcmp(dataType,"SInt")==0 ||
			strcmp(dataType,"DWord")==0 || strcmp(dataType,"Word")==0)
	{
		int value = 0;
		PyArg_Parse(py_data, "i", &value);
		if(dataSize == sizeof(int))
		{
			memcpy(data,&value,sizeof(value));
			return 0;
		}
		else
		{
			puts("dataSize err:int");
			return -1;
		}
	}
	else if(strcmp(dataType,"USInt")==0 || strcmp(dataType,"UInt")==0 || strcmp(dataType,"UDInt")==0)
	{
		unsigned int value = 0;
		PyArg_Parse(py_data, "I", &value);
		if(dataSize == sizeof(unsigned int))
		{
			memcpy(data,&value,sizeof(value));
			return 0;
		}
		else
		{
			puts("dataSize err:uint");
			return -1;
		}
	}
	else if(strcmp(dataType,"LInt")==0 || strcmp(dataType,"LWord")==0)
	{
		long value = 0;
		PyArg_Parse(py_data, "l", &value);
		if(dataSize == sizeof(long))
		{
			memcpy(data,&value,sizeof(value));
			return 0;
		}
		else
		{
			puts("dataSize err:long");
			return -1;
		}
	}
	else if(strcmp(dataType,"ULInt")==0)
	{
		unsigned long value = 0;
		PyArg_Parse(py_data, "k", &value);
		if(dataSize == sizeof(unsigned long))
		{
			memcpy(data,&value,sizeof(value));
			return 0;
		}
		else
		{
			puts("dataSize err:ulong");
			return -1;
		}
	}
	else if(strcmp(dataType,"Real")==0)
	{
		float value = 0;
		PyArg_Parse(py_data, "f", &value);
		if(dataSize == sizeof(float))
		{
			memcpy(data,&value,sizeof(value));
			return 0;
		}
		else
		{
			puts("dataSize err:float");
			return -1;
		}
	}
	else if(strcmp(dataType,"LReal")==0)
	{
		double value = 0;
		PyArg_Parse(py_data, "d", &value);
		if(dataSize == sizeof(double))
		{
			memcpy(data,&value,sizeof(value));
			return 0;
		}
		else
		{
			puts("dataSize err:double");
			return -1;
		}
	}
	else if(strcmp(dataType,"String")==0 || strcmp(dataType,"WString")==0)
	{
		char *value;
		PyArg_Parse(py_data, "s", &value);
		if(dataSize > strlen(value))
		{
			memcpy(data,value,strlen(value)+1);
			return 0;
		}
		else
		{
			puts("dataSize err:string");
			return -1;
		}
	}
	else
	{
		puts("dataType err");
		return -1;
	}
}


void SIEMENS_S7_1500_Read_And_Send(void)
{
	int ret = 0;
	if(plc_connect_flag == disconnected)
	{
		client = create_plc_client();
		//ret = connect_plc(client,"192.168.1.2",0,1);
		ret = connect_plc(client,plcGatewayCfgInfo.plcIp,
				plcGatewayCfgInfo.plcRack,plcGatewayCfgInfo.plcSlot);
		if(ret == 0)
		{
			plc_connect_flag = connected;
			puts("plc connected");
		}
		else
		{
			plc_connect_flag = disconnected;
			destroy_plc_client(client);
			puts("plc disconnected");
			return;
		}
	}

	memset(plcPointValue_buffer,0,sizeof(plcPointValue_buffer));
	for(int i = 0; i < plcPointTable.pointTableNumber; i++)
	{
		if(i!=0)
		{
			strcat(plcPointValue_buffer,",");
		}

		char address[PLC_POINT_ADDRESS_MAX_LEN+1] = {0};
		memcpy(address,plcPointTable.pointTable[i].address,sizeof(address));

		if(memcmp(address,"DB",2) == 0)
		{
			char dataType[PLC_POINT_DATATYPE_MAX_LEN+1] = {0};
			memcpy(dataType,plcPointTable.pointTable[i].datatype,sizeof(dataType));

			if(strcmp(dataType,"Bool")==0 || strcmp(dataType,"Byte")==0 ||
					strcmp(dataType,"Int")==0 || strcmp(dataType,"DInt")==0 || strcmp(dataType,"SInt")==0 ||
					strcmp(dataType,"DWord")==0 || strcmp(dataType,"Word")==0)
			{
				int value = 0;
				ret = readDB_plc(client,plcPointTable.pointTable[i].address,plcPointTable.pointTable[i].deviation,
						plcPointTable.pointTable[i].datatype,sizeof(value),&value);
				if(ret == 0)
				{
					sprintf(plcPointValue_buffer+strlen(plcPointValue_buffer),"%d",value);
				}
				else
				{
					puts("readDB_plc return err:int");
					break;
				}
			}
			else if(strcmp(dataType,"USInt")==0 || strcmp(dataType,"UInt")==0 || strcmp(dataType,"UDInt")==0)
			{
				unsigned int value = 0;
				ret = readDB_plc(client,plcPointTable.pointTable[i].address,plcPointTable.pointTable[i].deviation,
						plcPointTable.pointTable[i].datatype,sizeof(value),&value);
				if(ret == 0)
				{
					sprintf(plcPointValue_buffer+strlen(plcPointValue_buffer),"%u",value);
				}
				else
				{
					puts("readDB_plc return err:uint");
					break;
				}
			}
			else if(strcmp(dataType,"LInt")==0 || strcmp(dataType,"LWord")==0)
			{
				long value = 0;
				ret = readDB_plc(client,plcPointTable.pointTable[i].address,plcPointTable.pointTable[i].deviation,
						plcPointTable.pointTable[i].datatype,sizeof(value),&value);
				if(ret == 0)
				{
					sprintf(plcPointValue_buffer+strlen(plcPointValue_buffer),"%ld",value);
				}
				else
				{
					puts("readDB_plc return err:long");
					break;
				}
			}
			else if(strcmp(dataType,"ULInt")==0)
			{
				unsigned long value = 0;
				ret = readDB_plc(client,plcPointTable.pointTable[i].address,plcPointTable.pointTable[i].deviation,
						plcPointTable.pointTable[i].datatype,sizeof(value),&value);
				if(ret == 0)
				{
					sprintf(plcPointValue_buffer+strlen(plcPointValue_buffer),"%lu",value);
				}
				else
				{
					puts("readDB_plc return err:ulong");
					break;
				}
			}
			else if(strcmp(dataType,"Real")==0)
			{
				float value = 0;
				ret = readDB_plc(client,plcPointTable.pointTable[i].address,plcPointTable.pointTable[i].deviation,
						plcPointTable.pointTable[i].datatype,sizeof(value),&value);
				if(ret == 0)
				{
					sprintf(plcPointValue_buffer+strlen(plcPointValue_buffer),"%f",value);
				}
				else
				{
					puts("readDB_plc return err:float");
					break;
				}
			}
			else if(strcmp(dataType,"LReal")==0)
			{
				double value = 0;
				ret = readDB_plc(client,plcPointTable.pointTable[i].address,plcPointTable.pointTable[i].deviation,
						plcPointTable.pointTable[i].datatype,sizeof(value),&value);
				if(ret == 0)
				{
					sprintf(plcPointValue_buffer+strlen(plcPointValue_buffer),"%lf",value);
				}
				else
				{
					puts("readDB_plc return err:double");
					break;
				}
			}
			else if(strcmp(dataType,"String")==0 || strcmp(dataType,"WString")==0)
			{
				char value[512] = {0};
				ret = readDB_plc(client,plcPointTable.pointTable[i].address,plcPointTable.pointTable[i].deviation,
						plcPointTable.pointTable[i].datatype,sizeof(value),&value);
				if(ret == 0)
				{
					sprintf(plcPointValue_buffer+strlen(plcPointValue_buffer),"%s",value);
				}
				else
				{
					puts("readDB_plc return err:string");
					break;
				}
			}
			else
			{
				puts("point table dataType err.");
				return;
			}
		}
		else
		{
			int value = 0;
			ret = read_plc(client,plcPointTable.pointTable[i].address,&value);
			if(ret == 0)
			{
				sprintf(plcPointValue_buffer+strlen(plcPointValue_buffer),"%d",value);

			}
			else
			{
				puts("read_plc return err.");
				break;
			}
		}
	}

	printf("plc msgQ send:%s\n",plcPointValue_buffer);

	if(ret == -1)
	{
		//disconnect_plc(client);
		destroy_plc_client(client);
		plc_connect_flag = disconnected;
		return;
	}

	msgQueueData_t msgData;
	memset(&msgData, 0, sizeof(msgQueueData_t));
	msgData.cmd = CMD_SEND_PLC_POINT_VALUE;
	msgData.len = strlen(plcPointValue_buffer)+1;
	memcpy(msgData.data, plcPointValue_buffer, msgData.len);
	msgQSendToQuarkioe(&msgData);
}

void SIEMENS_S7_1500_End()
{
	Py_Finalize();
}


