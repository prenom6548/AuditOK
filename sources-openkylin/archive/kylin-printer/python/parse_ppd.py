#  Copyright (C) 2020, KylinSoft Co., Ltd.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.


import cups
import cupshelpers 
import os
import locale
import csv
# help(cups)

# new_locale = 'en_US.UTF-8'  # 替换为你希望设置的地区
# locale.setlocale(locale.LC_ALL, new_locale)

conn = cups.Connection()
printer = cupshelpers.Printer ('test',conn)
ppd = printer.getPPD()
oldppd = ppd

list=[
    'FormatVersion',
    'FileVersion',
    'LanguageVersion',
    'LanguageEncoding',
    'PCFileName',
    'Manufacturer',
    'ModelName',
    'ShortNickName',
    'NickName',
    'PSVersion',
    'LanguageLevel',
    'Font',
    'DefaultFont'
]

# for option in ppd.attributes:
    # if option.name not in list:
    #     # print(option.choices)
    #     print(option.spec)
    #     print(option.text)
    #     print(option.value)
    #     print("------------------------------------------------")

old_list=[]

for group in oldppd.optionGroups:
    for nr, option in enumerate(group.options):
        old_list.append(option.text)


        for choice in option.choices:
            old_list.append(choice['text'])

new_list=[]
ppd.localize()
for group in ppd.optionGroups:
    for nr, option in enumerate(group.options):
        new_list.append(option.text)

        for choice in option.choices:
            new_list.append(choice['text'])


dict={}
for nr, item in enumerate(new_list):
    key = old_list[nr]
    value=''
    if (new_list[nr] != old_list[nr]):
        value = new_list[nr]

    if key in dict:
        if dict[key] == '':
            dict[key] = value
    else:
        dict[key] = value
            


with open('out.csv', 'w', newline='') as file:
    writer = csv.writer(file, delimiter=',')

    for key, value in dict.items():
         writer.writerow([key,value])