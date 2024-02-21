BULK INSERT Directory
FROM 'D:\import SQL\mydir.txt'
WITH
(
    FORMATFILE = 'D:\import SQL\directories.fmt',  -- ��ʽ�ļ�·��
    FIRSTROW = 2 ,  -- ��һ���Ǳ���
	FIELDTERMINATOR = ',',  -- �ֶηָ���
    ROWTERMINATOR = '\n'    -- ����ֹ��
);
BULK INSERT Files
FROM 'D:\import SQL\myfile.txt'
WITH
(
    FORMATFILE = 'D:\import SQL\files.fmt',  -- ��ʽ�ļ�·��
    FIRSTROW = 2 , -- ��һ���Ǳ���
	FIELDTERMINATOR = ',',  -- �ֶηָ���
    ROWTERMINATOR = '\n'    -- ����ֹ��
);
