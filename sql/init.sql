CREATE DATABASE IF NOT EXISTS uuk_oj CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;
USE uuk_oj;
DROP TABLE IF EXISTS users;
CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(64) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    created_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
DROP TABLE IF EXISTS problems;
CREATE TABLE problems(
    id INT AUTO_INCREMENT PRIMARY KEY,
    title VARCHAR(255) NOT NULL,
    time_limit INT DEFAULT 1000,
    mem_limit INT DEFAULT 65536,
    created_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    description TEXT,
    sample_input TEXT,
    sample_output TEXT,
    tc_path VARCHAR(512),
    sub_count BIGINT DEFAULT 0,
    ac_count BIGINT DEFAULT 0
);
DROP TABLE IF EXISTS submissions;
CREATE TABLE submissions(
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    problem_id INT NOT NULL,
    user_id INT NOT NULL,
    language VARCHAR(32) NOT NULL,
    code TEXT,
    status VARCHAR(32) DEFAULT 'Pending',
    detail TEXT,
    submit_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    time_cost INT,
    mem_cost INT,
    INDEX(user_id,problem_id)
);
ALTER TABLE users
ADD COLUMN role TINYINT NOT NULL DEFAULT 0;
CREATE TABLE sessions (
    token CHAR(64) PRIMARY KEY,
    user_id INT NOT NULL,
    expire_time TIMESTAMP NULL,
    created_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX(user_id)
);
INSERT INTO users (username, password_hash, role) 
VALUES ('admin', 'b5142cebae610a79b387121208c5566426f0e97c2fe0d49f210e8040499b4aa0', 1);