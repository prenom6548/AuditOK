function(target_link_snmp_libraries prj_name)
    find_library(NETSNMPAGENT "netsnmpagent")
    find_library(NETSNMPMIBS "netsnmpmibs")
    find_library(NETSNMP "netsnmp")
    target_link_libraries(${prj_name} ${NETSNMPAGENT} ${NETSNMPMIBS} ${NETSNMP})
endfunction()

function(target_link_cups_libraries prj_name)
    target_link_libraries(${prj_name} PRIVATE "-lcups")
endfunction()

function(target_link_cupsfilters_libraries prj_name)
    target_link_libraries(${prj_name} PRIVATE "-lcupsfilters")
endfunction()

function(target_link_avahi_libraries prj_name)
    target_link_libraries(${prj_name} PRIVATE "-lavahi-common -lavahi-client")
endfunction()

function(target_link_usb_libraries prj_name)
    target_link_libraries(${prj_name} PRIVATE "-lusb-1.0")
endfunction()

function(target_link_udev_libraries prj_name)
    target_link_libraries(${prj_name} PRIVATE "-ludev")
endfunction()