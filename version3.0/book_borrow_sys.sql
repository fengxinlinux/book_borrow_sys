CREATE DATABASE book_borrow_sys;

use book_borrow_sys;

CREATE TABLE accounts
(
    number      VARCHAR(100) PRIMARY KEY,
    passwd      VARCHAR(20),
    nickname    VARCHAR(100),
    sex         VARCHAR(20),
    address     VARCHAR(100),
    birthdate   VARCHAR(20),
    phone       VARCHAR(20)
 );

CREATE TABLE book_infor
(
    ISBN          VARCHAR(20) PRIMARY KEY,
    book_name     VARCHAR(100),
    publish_house VARCHAR(100),
    author        VARCHAR(100),
    count         INT(11),
    stat          VARCHAR(20)
 );
 
CREATE TABLE borrow_infor
(
    account          VARCHAR(20),
    ISBN             VARCHAR(20),
    book_name        VARCHAR(100),
    borrow_date      DATETIME,
    ret_date         DATETIME,
    CONSTRAINT borrow_fk FOREIGN KEY (ISBN) REFERENCES book_infor(ISBN)
 );

CREATE TABLE ret_infor
(
    account    VARCHAR(20), 
    ISBN       VARCHAR(20),
    book_name  VARCHAR(100),
    return_date   DATETIME
);

