proc have_platform_drv {} {
	return [expr [have_spec platform_arndale] || [have_spec platform_imx53] || [have_spec platform_rpi]]
}

proc append_platform_drv_build_components {} {
	global build_components

	lappend_if [have_platform_drv] build_components drivers/platform
	lappend_if [have_spec acpi]    build_components drivers/acpi
	lappend_if [have_spec acpi]    build_components server/report_rom
	lappend_if [have_spec pci]     build_components drivers/pci
}

proc append_platform_drv_boot_modules {} {
	global boot_modules

	lappend_if [have_platform_drv] boot_modules platform_drv
	lappend_if [have_spec acpi]    boot_modules acpi_drv
	lappend_if [have_spec acpi]    boot_modules report_rom
	lappend_if [have_spec pci]     boot_modules pci_drv
	lappend_if [have_spec nova]    boot_modules pci_device_pd
}

proc platform_drv_policy {} {

	return {
			<policy label="ps2_drv">  <device name="PS2"/> </policy>
			<policy label="nic_drv">  <pci class="ETHERNET"/> </policy>
			<policy label="fb_drv">   <pci class="VGA"/> </policy>
			<policy label="wifi_drv"> <pci class="WIFI"/> </policy>
			<policy label="usb_drv">  <pci class="USB"/> </policy>
			<policy label="ahci_drv"> <pci class="AHCI"/> </policy>}
}

proc platform_drv_priority {} { return "" }

proc append_platform_drv_config {} {
	global config

	if {[have_platform_drv]} {
		append config {
	<start name="platform_drv">
		<resource name="RAM" quantum="1M" />
		<provides>
			<service name="Regulator"/>
			<service name="Platform"/>
		</provides>
	</start>}

	}

	if {[have_spec acpi]} {

		append config "
	<start name=\"acpi_drv\" [platform_drv_priority]>"

		append config {
		<resource name="RAM" quantum="2M"/>
		<route>
			<service name="LOG"> <parent/> </service>
			<service name="IO_MEM"> <parent/> </service>
			<service name="RM"> <parent/> </service>
			<service name="Report"> <child name="acpi_report_rom"/> </service>
		</route>
	</start>}

		append config "
	<start name=\"acpi_report_rom\" [platform_drv_priority]>"

		append config {
		<binary name="report_rom"/>
		<resource name="RAM" quantum="2M"/>
		<provides>
			<service name="ROM" />
			<service name="Report" />
		</provides>
		<config>
			<rom> <policy label="pci_drv -> acpi" report="acpi_drv -> acpi"/> </rom>
		</config>
		<route>
			<service name="LOG"> <parent/> </service>
			<service name="SIGNAL"> <parent/> </service>
			<service name="CAP"> <parent/> </service>
			<service name="RM"> <parent/> </service>
		</route>
	</start>}

	}

	if {[have_spec pci]} {

		append config "
	<start name=\"pci_drv\" [platform_drv_priority]>"

		append config {
		<resource name="RAM" quantum="3M" constrain_phys="yes"/>
		<provides> <service name="PCI"/> </provides>
		<route>}

		if {[have_spec acpi]} {
			append config {
			<service name="ROM">
				<if-arg key="label" value="acpi"/> <child name="acpi_report_rom"/>
			</service>}
		}

		append config {
			<any-service> <parent/> </any-service>
		</route>}

		if {[have_spec acpi]} {

		append config {
		<config>}

		} else {

		append config {
		<config acpi="no">}

		}

		append config [platform_drv_policy]

		append config {
		</config>
	</start>}

	}

	return $config
}
