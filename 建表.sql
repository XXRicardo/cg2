-- 创建目录表
CREATE TABLE Directory (
    name NVARCHAR(255) NOT NULL,
    depth INT NOT NULL,
    file_count INT NOT NULL,
    parent_directory NVARCHAR(255)
);

-- 创建文件表
CREATE TABLE Files (
    filename NVARCHAR(255) NOT NULL,
    path NVARCHAR(255) NOT NULL,
    file_size BIGINT NOT NULL,
    last_write_time BIGINT NOT NULL,
    depth INT NOT NULL,
);

