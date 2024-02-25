BULK INSERT Directory
FROM 'D:\import SQL\mydir.txt'
WITH
(
    FORMATFILE = 'D:\import SQL\directories.fmt',  -- 格式文件路径
    FIRSTROW = 2 ,  -- 第一行是标题
	FIELDTERMINATOR = ',',  -- 字段分隔符
    ROWTERMINATOR = '\n'    -- 行终止符
);
BULK INSERT Files
FROM 'D:\import SQL\myfile.txt'
WITH
(
    FORMATFILE = 'D:\import SQL\files.fmt',  -- 格式文件路径
    FIRSTROW = 2 , -- 第一行是标题
	FIELDTERMINATOR = ',',  -- 字段分隔符
    ROWTERMINATOR = '\n'    -- 行终止符
);
