drop table if exists users;
create table users (id integer primary key autoincrement, login text, password text);
create unique index u_login on users (login);
insert into users (login,password) values ('adam','adam');
insert into users (login,password) values ('bozena','bozena');
insert into users (login,password) values ('celina','celina');