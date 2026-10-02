## Storage组

### 工作目标

本SIG组致力于以下工作：

1.创建并维护存储SIG-Storage社区
	1）维护社区Linux Storage I/O Stack 全部软件栈
	2）为SCSI/NVMe/FC 等存储协议与相关存储芯片硬件提供适配与兼容性测试

2.为基于麒麟OS的存储系统提供软硬件的适配与支持
	1）帮助社区完成NVMe/SCSI/UFS等高速存储协议从内核态转向用户态的技术迭代
	2）针对LSI/PMC/Qlogic 等主流HBA/RAID芯片的SCSI与NVMe驱动维护与支持

3.块设备、文件系统等通用存储软件栈的维护与软硬件适配
	1）帮助社区维护块设备与文件系统等通用存储I/O软件栈与主流数据库等系统软件的适配；
	2）帮助社区为超融合、软件定义存储、分布式存储等存储软硬件系统集成提供存储I/O栈的代码维护与兼容性测试

### SIG Owner
- leap-io@kylinos.cn

### SIG成员
- lichuang@kylinos.cn

### SIG Maintainer
- lichuang@kylinos.cn

### SIG维护包列表
- drivers/nvme :
	nvme-core.ko
	nvme-fabrics.ko
	nvme-fc.ko
	nvme.ko
	nvme-rdma.ko
	nvme-tcp.ko
	nvme-scsi.ko

- drivers/nvdimm :
	nd_btt.ko
	nd_pmem.ko
	nd_virtio.ko
	virtio_pmem.ko

- drivers/nvmem :
	nvmem_qcom-spmi-sdam.ko
	nvmem-rmem.ko

- drivers/md :
	dm-bio-prison.ko
	dm-bufio.ko
	dm-cache.ko
	dm-cache-smq.ko
	dm-clone.ko
	dm-crypt.ko
	dm-delay.ko
	dm-ebs.ko
	dm-era.ko
	dm-flakey.ko
	dm-historical-service-time.ko
	dm-integrity.ko
	dm-io-affinity.ko
	dm-log.ko
	dm-log-userspace.ko
	dm-log-writes.ko
	dm-mirror.ko
	dm-multipath.ko
	dm-queue-length.ko
	dm-raid.ko
	dm-region-hash.ko
	dm-round-robin.ko
	dm-service-time.ko
	dm-snapshot.ko
	dm-switch.ko
	dm-thin-pool.ko
	dm-unstripe.ko
	dm-verity.ko
	dm-writecache.ko
	dm-zero.ko
	dm-zoned.ko
	faulty.ko
	linear.ko
	md-cluster.ko
	multipath.ko
	raid0.ko
	raid10.ko
	raid1.ko
	raid456.ko

- drivers/ata :
	acard-ahci.ko
	ahci.ko
	ahci_platform.ko
	libahci.ko
	libahci_platform.ko
	pata_acpi.ko
	pata_ali.ko
	pata_amd.ko
	pata_artop.ko
	pata_atiixp.ko
	pata_atp867x.ko
	pata_cmd640.ko
	pata_cmd64x.ko
	pata_cypress.ko
	pata_efar.ko
	pata_hpt366.ko
	pata_hpt37x.ko
	pata_hpt3x2n.ko
	pata_hpt3x3.ko
	pata_it8213.ko
	pata_it821x.ko
	pata_jmicron.ko
	pata_legacy.ko
	pata_marvell.ko
	pata_mpiix.ko
	pata_netcell.ko
	pata_ninja32.ko
	pata_ns87410.ko
	pata_ns87415.ko
	pata_oldpiix.ko
	pata_optidma.ko
	pata_opti.ko
	pata_pcmcia.ko
	pata_pdc2027x.ko
	pata_pdc202xx_old.ko
	pata_piccolo.ko
	pata_radisys.ko
	pata_rdc.ko
	pata_rz1000.ko
	pata_sch.ko
	pata_serverworks.ko
	pata_sil680.ko
	pata_sl82c105.ko
	pata_triflex.ko
	pata_via.ko
	pdc_adma.ko
	sata_dwc_460ex.ko
	sata_inic162x.ko
	sata_mv.ko
	sata_nv.ko
	sata_promise.ko
	sata_qstor.ko
	sata_sil24.ko
	sata_sil.ko
	sata_sis.ko
	sata_svw.ko
	sata_sx4.ko
	sata_uli.ko
	sata_via.ko
	sata_vsc.ko

- drivers/scsi :
	libsas.ko
	aacraid.ko
	mpt3sas.ko
	libfc.ko
	pm80xx.ko
	megaraid_mbox.ko
	megaraid_mm.ko
	megaraid_sas.ko
	3w-9xxx.ko
	3w-sas.ko
	3w-xxxx.ko
	53c700.ko
	a100u2w.ko
	advansys.ko
	aha1740.ko
	am53c974.ko
	atp870u.ko
	BusLogic.ko
	ch.ko
	dc395x.ko
	dmx3191d.ko
	esp_scsi.ko
	fdomain.ko
	fdomain_pci.ko
	hpsa.ko
	hptiop.ko
	hv_storvsc.ko
	imm.ko
	initio.ko
	ipr.ko
	ips.ko
	iscsi_boot_sysfs.ko
	iscsi_tcp.ko
	libiscsi.ko
	libiscsi_tcp.ko
	megaraid.ko
	mvumi.ko
	myrb.ko
	myrs.ko
	pmcraid.ko
	ppa.ko
	qla1280.ko
	qlogicfas408.ko
	raid_class.ko
	scsi_debug.ko
	scsi_transport_fc.ko
	scsi_transport_iscsi.ko
	scsi_transport_sas.ko
	scsi_transport_spi.ko
	scsi_transport_srp.ko
	ses.ko
	sim710.ko
	stex.ko
	st.ko
	virtio_scsi.ko
	vmw_pvscsi.ko
	wd719x.ko
	xen-scsifront.ko

- fs/ufs :
	ufs.ko
