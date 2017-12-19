/*!40101 SET NAMES utf8 */;

CREATE TABLE HLR_USERID_TELNO_REL 
(
	USERID VARCHAR(9) not null,
	NATIONAL_CODE VARCHAR(6) not null,
	TELNO VARCHAR(33) not null,
	TOTAL_TELNO VARCHAR(33) not null,
	TELNO_TYPE int default 0,
	CONSTRAINT  PK_HLR_USERID_TELNO_REL PRIMARY KEY (NATIONAL_CODE, TELNO)
);

alter table HLR_USERID_TELNO_REL comment '电话号码和UID绑定关系表';

create table UDC_UID_LOCATION
( 
  USERID         VARCHAR(9) not null,
  NATIONAL_CODE  VARCHAR(6),
  TELNO          VARCHAR(33),
  NETWORKID      VARCHAR(10) default '0',
  CONSTRAINT  PK_UDC_UID_LOCATION PRIMARY KEY (USERID)
);

alter table UDC_UID_LOCATION comment '用户归属位置表';

CREATE TABLE LEASEHOLD_CONTROL 
(
	PID VARCHAR(10) not null,
	PORT int default 0,
	CREDIT_USERID VARCHAR(10),
	BIND_USERID VARCHAR(10),
	TERMINAL_STATUS int default 0,
	CREDIT_STATUS int DEFAULT 0,
	CONSTRAINT  PK_LEASEHOLD_CONTROL PRIMARY KEY (PID,PORT)
);

alter table LEASEHOLD_CONTROL comment '终端租赁绑定关系表';


create index index_HLR_USERID_TELNO_REL on HLR_USERID_TELNO_REL(TOTAL_TELNO);
create index index_UDC_UID_LOCATION on UDC_UID_LOCATION(USERID);
create index index_LEASEHOLD_CONTROL on LEASEHOLD_CONTROL(PID, PORT);