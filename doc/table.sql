/*
 * Tables used in gtkqq
 */

/*
 * config table
 * contain all the configuration items.
 * The key is string. The value can be any type according to the key.
 */
create table if not exists config(
    id primary key asc autoinscrement,  -- primary key int
    owner,                              -- the owner. string
    key,                                -- key. string
    value                               -- value. string
);

/*
 * qquser table
 * All the users who has logined
 * `last` is used to mark the last login user.
 */
create table if not exists qquser(
    qqnumber primary key ,              -- primary key string
    last,                               -- int
    passwd,                             -- base64 encoded password
    status                              -- last status
);

/*
 * buddies table
 * all the buddies
 */
create table if not exists buddies(
    id primary key asc autoinscrement,  -- primary key int
    owner,              -- the owner of this buddy. string
    qqnumber,           -- string
    vip_info,           -- int
    nick,               -- string
    markname,           -- string
    faceimgfile,        -- the file path of the face image. string
    country,            -- string
    province,           -- string
    city,               -- string
    gender,             -- int
    face,               -- int
    flag,               -- int
    birthday_y,         -- birthday year. int
    birthday_m,         -- birthday month. int
    birthday_d,         -- birthday day. int
    blood,              -- int
    shengxiao,          -- int
    constel,            -- int
    phone,              -- string
    mobile,             -- string
    email,              -- string
    occupation,         -- string
    college,            -- string
    homepage,           -- string
    personal,           -- string
    lnick,              -- string
    cate_idx,           -- the index of catogory which this buddy is belong to.

    -- foreign key
    foreign key(owner) references qquser(qqnumber) on delete cascade 
);

/*
 * groups table
 */
create table if not exists groups(
    gnumber primary key,    -- group number. string
    owner,                  -- the owner of this buddy. string
    name,                   -- string
    code,                   -- string
    flag,                   -- string
    creater,                -- string
    mark,                   -- string
    mask,                   -- string
    opt,                    -- int
    createtime,             -- string
    gclass,                 -- int
    glevel,                 -- int
    face,                   -- int
    memo,                   -- string
    fingermemo              -- sttring

    -- foreign key
    foreign key(owner) references qquser(qqnumber) on delete cascade 
);

/*
 * group member table
 */
create table if not exists gmemebers(
    gnumber,                -- the group number. string
    qqnumber,               -- qq number. string
    nick,                   -- string
    flag,                   -- string
    card                    -- string

    -- foreign key
    foreign key(gnumber) references groups(gnumber) on delete cascade 
);

/*
 * categories table
 */
create table if not exists categories(
    id primary key asc autoinscrement,  -- primary key int
    owner,                              -- the owner of this buddy. string
    idx,                                -- index. int
    name,                               -- name. string

    -- foreign key
    foreign key(owner) references qquser(qqnumber) on delete cascade 
);

