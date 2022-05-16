#===========================================================================================#
#                                  更新厂商 MAC 信息表                                      #
#===========================================================================================#
# 从 web 上下载下来的所以厂商的 MAC 表
VENDOR_TABLES_FILE = all_vendor.txt
# 要生成的目标厂商 MAC 信息的目标文件
VENDOR_TARGET_FILE = mac_vendor.txt
# 单个厂商的临时格式的 MAC 信息(可能含有杂乱信息，需要过滤)
VENDOR_TEMP_FILE = tmp.txt
# 把除 MAC 信息外的其他检索出来的字符串过滤后的信息
VENDOR_FILTER_FILE = filter.txt
# MAC 表中，MAC 信息所在行的关键字
VENDOR_MAC_KEYWORDS = "base 16"

# 要获取厂商的名字(忽略大小写)
# 为保证填写的厂商名字正确，请查看文件 https://standards-oui.ieee.org/oui/oui.txt
VENDOR_NAME += huawei honor tenda xiaomi oppo vivo apple
VENDOR_NAME += hikvision ezviz skyworth tecno panasonic sharp
VENDOR_NAME += haier vizio gosund arlo belkin samsung htc lg
VENDOR_NAME += letv meizu lenovo sony acer nokia asus motorola
VENDOR_NAME += zte google hisense tcl gionee amazon oneplus
VENDOR_NAME += philips tp-link realme

all: vendor_update

vendor_update: vendor_init vendor_mac_info_get vendor_clean

# 获取所有厂商的 MAC 信息txt，并重命名
vendor_init:
	wget -O./$(VENDOR_TABLES_FILE) "https://standards-oui.ieee.org/oui/oui.txt"

# MAC信息格式 ==> 厂商名:MAC列表 (一行只写25个MAC，便于读取)
vendor_mac_info_get:
	@rm -rf $(VENDOR_TARGET_FILE)
	@for i in $(VENDOR_NAME) ; \
	do \
		grep -wi -B 1 $$i $(VENDOR_TABLES_FILE) | grep -i $(VENDOR_MAC_KEYWORDS) | awk '{printf $$1 "\n"}' > $(VENDOR_TEMP_FILE); \
        awk '{print length(), $$0}' $(VENDOR_TEMP_FILE) | grep "6" | awk '{printf $$2 "\n"}' | grep -iv $$i > $(VENDOR_FILTER_FILE); \
		awk -v name=$$i '{if(NR % 25 == 1) {printf("%s:%s ", name, $$0)} else if(NR % 25 != 0) {printf("%s ", $$0)} else {print $$0}}' $(VENDOR_FILTER_FILE) >> $(VENDOR_TARGET_FILE); \
		echo -e "\n" >> $(VENDOR_TARGET_FILE); \
    done
# 获取 coolpad 厂商的 MAC 池，coolpad MAC信息比较乱，需特殊处理
	@grep -wi -B 1 "coolpad" $(VENDOR_TABLES_FILE)| grep -i $(VENDOR_MAC_KEYWORDS) | awk '{printf $$1 "\n"}' > $(VENDOR_TEMP_FILE);
	@grep -wi -B 1 "yulong" $(VENDOR_TABLES_FILE)| grep -i $(VENDOR_MAC_KEYWORDS) | grep -iv "realme" | awk '{printf $$1 "\n"}' >> $(VENDOR_TEMP_FILE);
	@awk '{print length(), $$0}' $(VENDOR_TEMP_FILE) | grep "6" | awk '{printf $$2 "\n"}' | grep -iv "yulong" > $(VENDOR_FILTER_FILE);
	@awk -v name=coolpad '{if(NR % 25 == 1) {printf("%s:%s ", name, $$0)} else if(NR % 25 != 0) {printf("%s ", $$0)} else {print $$0}}' $(VENDOR_FILTER_FILE) >> $(VENDOR_TARGET_FILE);
	@echo -e "\n" >> $(VENDOR_TARGET_FILE);

# 将临时文件清除
vendor_clean:
	@rm -rf $(VENDOR_TEMP_FILE) $(VENDOR_TABLES_FILE) $(VENDOR_FILTER_FILE)