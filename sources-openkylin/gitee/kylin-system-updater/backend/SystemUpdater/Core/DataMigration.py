#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
import logging
import sqlite3
from operator import itemgetter
from gettext import gettext as _
from optparse import OptionParser

DB_UPDATER = "/var/cache/kylin-update-manager/kylin-update-manager.db"
DB_UPGRADE = "/var/cache/kylin-system-updater/kylin-system-updater.db"
VER_DB = "/usr/share/kylin-system-updater/kylin-system-updater.db"

def dateMigration(options=None, old_db=None, old_db_cursor=None, new_db=None, new_db_cursor=None):
	print(_("Loading Sqlite3Server..."))
	if options==None:
		old_path = DB_UPDATER
		new_path = DB_UPGRADE
	try:
		if old_db==None and old_db_cursor==None:
			old_db = sqlite3.connect(old_path, check_same_thread=False)
			old_db_cursor = old_db.cursor()
		if new_db==None and new_db_cursor==None:
			new_db = sqlite3.connect(new_path, check_same_thread=False)
			new_db_cursor = new_db.cursor()
	except Exception as e:
		print(e)

	sql_commnd = ""
	old_cfg_dict = {}
	new_cfg_dict = {}
	# step 1: 更新旧配置数据
	try:
		print("更新旧配置数据")
		sql_commnd = "SELECT * FROM display where id=1"
		old_db_cursor.execute(sql_commnd)
		old_cfg = old_db_cursor.fetchone()
		for od in old_db_cursor.description:
			old_cfg_dict.update({str(od[0]):old_cfg[old_db_cursor.description.index(od)]})
		new_db_cursor.execute(sql_commnd)
		new_cfg = new_db_cursor.fetchone()
		for od in new_db_cursor.description:
			new_cfg_dict.update({str(od[0]):new_cfg[new_db_cursor.description.index(od)]})

		if "download_limit" in new_cfg_dict.keys() and "download_limit_value" in new_cfg_dict.keys():
			if new_cfg_dict['download_limit'] != None or new_cfg_dict['download_limit_value'] != None:
				print("目标数据库有更新的配置项")
		else:
			sql_commnd = "UPDATE display set check_time='"+old_cfg_dict['check_time']+"' Where id=1"
			new_db_cursor.execute(sql_commnd)
			new_db.commit()
			sql_commnd = "UPDATE display set update_time='"+old_cfg_dict['update_time']+"' Where id=1"
			new_db_cursor.execute(sql_commnd)
			new_db.commit()
			sql_commnd = "UPDATE display set auto_check='"+old_cfg_dict['auto_check']+"' Where id=1"
			new_db_cursor.execute(sql_commnd)
			new_db.commit()
			sql_commnd = "UPDATE display set system_version='"+old_cfg_dict['system_version']+"' Where id=1"
			new_db_cursor.execute(sql_commnd)
			new_db.commit()
			if old_cfg_dict['auto_backup'] != None:
				sql_commnd = "UPDATE display set auto_backup='"+old_cfg_dict['auto_backup']+"' Where id=1"
				new_db_cursor.execute(sql_commnd)
				new_db.commit()
			if 'download_limit' in old_cfg_dict.keys() and old_cfg_dict['download_limit'] != None:
				sql_commnd = "UPDATE display set download_limit='"+old_cfg_dict['download_limit']+"' Where id=1"
				new_db_cursor.execute(sql_commnd)
				new_db.commit()
			if 'download_limit_value' in old_cfg_dict.keys() and old_cfg_dict['download_limit_value'] != None:
				sql_commnd = "UPDATE display set download_limit_value='"+old_cfg_dict['download_limit_value']+"' Where id=1"
				new_db_cursor.execute(sql_commnd)
				new_db.commit()

	except Exception as e:
		print(e)
		print("更新配置文件错误")
		return 

  	# step 2: 更新installed
	try:
		print("更新installed")
		update_record_dict = {}
		tmp_update_record_dict = []
		sql_commnd = "SELECT * FROM installed"
		old_db_cursor.execute(sql_commnd)
		update_record = old_db_cursor.fetchall()
		sql_commnd = "SELECT * FROM updateinfos"
		new_db_cursor.execute(sql_commnd)
		new_update_record = new_db_cursor.fetchall()
    
		for ur in update_record:
			id,appname,version,time,description,icon,statue,keyword,errorcode = ur
			if errorcode in range(200):
				errorcode = 'null'
			update_record_dict.clear()
			update_record_dict.update({"appname":appname})
			update_record_dict.update({"version":version})
			update_record_dict.update({"time":time})
			update_record_dict.update({"description":description})
			update_record_dict.update({"icon":icon})
			update_record_dict.update({"statue":statue})
			update_record_dict.update({"keyword":'1'})
			update_record_dict.update({"errorcode":errorcode})
			tmp_update_record_dict.append(update_record_dict.copy())
		for ur in new_update_record:
			id,appname,version,description,date,status,keyword,errorcode = ur
			if errorcode in range(200):
				errorcode = 'null'
			update_record_dict.clear()
			update_record_dict.update({"appname":appname})
			update_record_dict.update({"version":version})
			update_record_dict.update({"time":date})
			update_record_dict.update({"description":description})
			update_record_dict.update({"icon":None})
			update_record_dict.update({"statue":status})
			update_record_dict.update({"keyword":'1'})
			update_record_dict.update({"errorcode":errorcode})
			tmp_update_record_dict.append(update_record_dict.copy())

		# 按时间排序
		tmp_update_record_dict = sorted(tmp_update_record_dict, key=itemgetter('time'))
		print("更新installed success")
    
	except Exception as e:
		print(e)
		print("更新安装记录错误")
		return

	try:
		# 删除 tmp
		# DeleteTable(options.new_path+':'+'tmp')
		# 创建表
		sql_commnd = "create table IF NOT EXISTS tmp('id' INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,\
		'appname'	TEXT,\
		'version'	TEXT,\
		'description'	TEXT,\
		'date'	TEXT,\
		'status'	TEXT,\
		'keyword'	TEXT,\
		'errorcode' TEXT) "
		new_db_cursor.execute(sql_commnd)
		# 更新数据
		for urd in tmp_update_record_dict:
			new_db_cursor.execute(
            "insert into tmp (appname, version, description, date, status, keyword, errorcode) values(?,"
            "?,?,?,?,?,?)",
            (urd['appname'], urd['version'], urd['description'], urd['time'], urd['statue'], urd['keyword'], urd['errorcode']))
			new_db.commit()
		
		# 删除updateinfos
		sql_commnd = "drop table updateinfos"
		new_db_cursor.execute(sql_commnd)
		new_db.commit()
		# 修改表名
		sql_commnd = "alter table tmp rename to updateinfos"
		new_db_cursor.execute(sql_commnd)
		new_db.commit()
	except Exception as e:
		print(e)
		print("安装记录迁移错误")
		return
	print("数据迁移成功.")

def CleanTable(db_table):
	db_path, table_name = str(db_table).split(":")
	if not os.path.isfile(db_path):
		print("db path error.")
		exit(-1)
	print(_("Loading Sqlite3Server..."))
	try:
		db = sqlite3.connect(db_path, check_same_thread=False)
		db_cursor = db.cursor()
		sql_commnd = 'delete from '+table_name
		db_cursor.execute(sql_commnd)
		db.commit()
		print("clean %s success."%table_name)
	except Exception as e:
		print(e)
		print("clean %s error."%table_name)

def DeleteTable(db_table):
	db_path, table_name = str(db_table).split(":")
	if not os.path.isfile(db_path):
		print("db path error.")
		exit(-1)
	print(_("Loading Sqlite3Server..."))
	try:
		db = sqlite3.connect(db_path, check_same_thread=False)
		db_cursor = db.cursor()
		sql_commnd = 'drop table '+table_name
		db_cursor.execute(sql_commnd)
		db.commit()
		print("delete %s success."%table_name)
	except Exception as e:
		print("delete %s error: %s"%(table_name,e))

def _has_first_migration(new_db, new_db_cursor):
	try:
		sql_commnd = "select * from sqlite_master where type='table' and name='display';"
		new_db_cursor.execute(sql_commnd)
		retval = new_db_cursor.fetchone()
		for rv in retval:
			if "firstmigration" in str(rv):
				return True
	except Exception as e:
		print(e)
		return False

def _is_first_migration(new_db, new_db_cursor):
	try:
		sql_commnd = "select firstmigration from display;"
		new_db_cursor.execute(sql_commnd)
		retval = new_db_cursor.fetchone()
		if "yes" in retval:
			return True
		else :
			return False
	except Exception as e:
		print(e)
		return False

def _is_display_exist_fields(field, db, db_cursor):
	try:
		sql_commnd = "select * from sqlite_master where type='table' and name='display';"
		db_cursor.execute(sql_commnd)
		retval = db_cursor.fetchone()
		for rv in retval:
			if field in str(rv):
				return True
	except Exception as e:
		print(e)
		return False
	return False

def _is_updateinfos_exist_fields(field, db, db_cursor):
	try:
		sql_commnd = "select * from sqlite_master where type='table' and name='updateinfos';"
		db_cursor.execute(sql_commnd)
		retval = db_cursor.fetchone()
		for rv in retval:
			if field in str(rv):
				return True
	except Exception as e:
		print(e)
		return False
	return False

def _add_display_fields(fields_default, default_table = True):
	try:
		if "=" not in fields_default:
			print("format: field=value")
			return False
		field, value = fields_default.split('=')
		# print(_("Loading Sqlite3Server..."))
		db = sqlite3.connect(DB_UPGRADE, check_same_thread=False)
		db_cursor = db.cursor()
		if default_table:
			if _is_display_exist_fields(field, db, db_cursor):
				print("field %s is exist."%field)
				return False
			# 字段不存在,新增字段
			sql_commnd = "alter table display add column "+field+" TEXT;"
			db_cursor.execute(sql_commnd)
			sql_commnd = "UPDATE display SET "+field+"='"+value+"'"
			db_cursor.execute(sql_commnd)
			db.commit()
		else:
			if _is_updateinfos_exist_fields(field, db, db_cursor):
				print("field %s is exist."%field)
				return False
			# 字段不存在,新增字段
			sql_commnd = "alter table updateinfos add column "+field+" TEXT;"
			db_cursor.execute(sql_commnd)
			db.commit()
	except Exception as e:
		print(e)
		return False
	print("Succeeded in adding field: '%s' "%field)
	return True

def _add_new_table(table):
	table = str(table).strip()
	if "=" not in table:
		return False
	opt, fields = table.split('=')
	try:
		if fields == 'tid_search':
			db = sqlite3.connect(DB_UPGRADE, check_same_thread=False)
			db_cursor = db.cursor()
			sql_commnd = "create table IF NOT EXISTS tid_search('id' INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,\
							'key'	TEXT,\
							'tid'	TEXT) "
			db_cursor.execute(sql_commnd)
			db.commit()
			db.close()
	except Exception as e:
		print(e)
		return False

def CopyData():
	try:
		# 判断新字段是否存在
		if (os.path.exists(VER_DB) and os.path.exists(DB_UPGRADE)):
			print(_("Loading Sqlite3Server..."))
			try:
				new_db = sqlite3.connect(DB_UPGRADE, check_same_thread=False)
				new_db_cursor = new_db.cursor()
				ver_db = sqlite3.connect(VER_DB, check_same_thread=False)
				ver_db_cursor = ver_db.cursor()
			except Exception as e:
				print(e)

			if (_has_first_migration(new_db, new_db_cursor)): # 存在 firstmigration
				if (_is_first_migration(new_db, new_db_cursor)): 
					# 数据迁移
					dateMigration(new_db=new_db, new_db_cursor=new_db_cursor)
					sql_commnd = "UPDATE display SET firstmigration='false';"
					new_db_cursor.execute(sql_commnd)
					new_db.commit()
				else:
					print("No data migration is required ...")
			else:# 不存在firstmigration
				# 新增  firstmigration 字段
				sql_commnd = "alter table display add column firstmigration text;"
				new_db_cursor.execute(sql_commnd)
				sql_commnd = "UPDATE display SET firstmigration='true';"
				new_db_cursor.execute(sql_commnd)
				#数据迁移
				dateMigration(new_db=new_db, new_db_cursor=new_db_cursor)
				sql_commnd = "UPDATE display SET firstmigration='false';"
				new_db_cursor.execute(sql_commnd)
				new_db.commit()

		else :
			print("Not found kylin-system-updater.db, ensure that \'kylin-system-updater\' is successfully installed ... ")
			exit(-1)
	except Exception as e:
		print(e)

if __name__ == "__main__":
	# Begin parsing of options
	parser = OptionParser()
	parser.add_option ("-d", "--debug", action="store_true", default=False,
                    help=_("Show debug messages"))
	parser.add_option ("-o", "--old-path", dest="old_path",
                    help=_("Enter the old database address"))
	parser.add_option ("-n", "--new-path", dest="new_path",
                    help=_("Enter the new database address"))
	parser.add_option ("-c", "--clean-table", dest="clean_table",
                    help=_("Clear the table"))
	parser.add_option ("-r", "--delete-table", dest="delete_table",
                    help=_("Delete the table"))
	parser.add_option ("-m", "--data-migration", default=False, action="store_true", 
					dest="data_migration", help=_("data migration"))
	parser.add_option ("-f", "--add-display-fields", 
					dest="add_display_fields", help=_("add display fields"))
	parser.add_option ("-u", "--add-updateinfos-fields", 
					dest="add_updateinfos_fields", help=_("add updateinfos fields"))
	parser.add_option ("-t", "--add-new-table", 
					dest="add_new_table", help=_("add new table"))
	(options, args) = parser.parse_args()

	if options.clean_table:
		if ":" not in options.clean_table:
			print("format error: <database:table>")
		else:
			CleanTable(str(options.clean_table))
	
	if options.delete_table:
		if ":" not in options.delete_table:
			print("format error: <database:table>")
		else:
			DeleteTable(str(options.delete_table))
	
	if options.add_display_fields:
		_add_display_fields(str(options.add_display_fields))
	
	if options.add_updateinfos_fields:
		_add_display_fields(str(options.add_updateinfos_fields), default_table = False)

	if options.add_new_table:
		_add_new_table(str(options.add_new_table))

	if options.data_migration:
		CopyData()
		exit(0)
  
	if options.old_path or options.new_path:
		# 检查文件
		if not options.old_path or not options.new_path:
			print("parameter error")
			exit(-1)
		if not os.path.isfile(options.old_path):
			print("The source database file does not exist")
			exit(-1)
		if not os.path.isfile(options.new_path):
			print("The destination database file does not exist")
			exit(-1)
		dateMigration(options)
