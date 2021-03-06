#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include <locale.h>
#include <unistd.h>
#include <time.h>

#include <bpf/bpf.h>
/* Lesson#1: this prog does not need to #include <bpf/libbpf.h> as it only uses
 * the simple bpf-syscall wrappers, defined in libbpf #include<bpf/bpf.h>
 */

#include <net/if.h>
#include <linux/if_link.h> /* depend on kernel-headers installed */

#include "../includes/common/common_params.h"
#include "../includes/common/common_user_bpf_xdp.h"
#include "common_kern_user.h"
#include "bpf_util.h" /* bpf_num_possible_cpus */

const char *pin_dir =  "/sys/fs/bpf/ovs-router";

struct lpm_val {
	__u8 dst_mac[6];
	__u8 out_of_port;	// output openflow port
};

int main(int argc, char **argv){
	if(argc < 4){
		printf("Usage: ./map_updater <mapname> <insert/update/delete/lookup> <key> [val]\n");\
		return 0;
	}

	struct bpf_map_info info = { 0 };
	int map_fd = open_bpf_map_file(pin_dir, argv[1], &info);
	if (map_fd < 0) return EXIT_FAIL_BPF;

	if(strcmp(argv[1], "counter") == 0 || strcmp(argv[1], "tx_port") == 0){
		int key, val;
		key = atoi(argv[3]);
		switch (argv[2][0]){
			case 'i':
			case 'u':
				if(argc != 5) return -1;
				val = atoi(argv[4]);
				bpf_map_update_elem(map_fd, &key, &val, 0);
				break;
			case 'd':
				bpf_map_delete_elem(map_fd, &key);
				break;
			case 'l':
				bpf_map_lookup_elem(map_fd, &key, &val);
				printf("value: %d\n", val);
				break;
		}
	}else if(strcmp(argv[1], "sw_nics") == 0){
		int key;
		__u8 val[6];
		key = atoi(argv[3]);
		switch (argv[2][0]){
			case 'i':
			case 'u':
				if(argc != 5) return -1;
				val[0] = (int)strtol( strtok(argv[4], ":") , NULL, 16);
				val[1] = (int)strtol( strtok(NULL, ":") , NULL, 16);
				val[2] = (int)strtol( strtok(NULL, ":") , NULL, 16);

				val[3] = (int)strtol( strtok(NULL, ":") , NULL, 16);
				val[4] = (int)strtol( strtok(NULL, ":") , NULL, 16);
				val[5] = (int)strtol( strtok(NULL, ":") , NULL, 16);

				bpf_map_update_elem(map_fd, &key, &val[0], 0);
				break;
			case 'd':
				bpf_map_delete_elem(map_fd, &key);
				break;
			case 'l':
				bpf_map_lookup_elem(map_fd, &key, &val[0]);
				printf("value: %02x:%02x:%02x : %02x:%02x:%02x\n", val[0], val[1], val[2], val[3], val[4], val[5]);
				break;
		}
	}else{
		union {
			__u32 b32[2];
			__u8 b8[8];
		} key4;
		struct lpm_val val;
		char *ip_addr, *prefix, *dst_mac, *out_of_port_str;
		ip_addr = strtok(argv[3], "/");
		if(ip_addr != NULL)
			prefix = strtok(NULL, "/");
		key4.b32[0] = atoi(prefix);
		key4.b8[4] = atoi( strtok(ip_addr, ".") );
		key4.b8[5] = atoi( strtok(NULL, ".") );
		key4.b8[6] = atoi( strtok(NULL, ".") );
		key4.b8[7] = atoi( strtok(NULL, ".") );
		if(argc >= 5 && argv[2][0] != 'l'){
			dst_mac = strtok(argv[4], "/");
			if(dst_mac != NULL){
				out_of_port_str = strtok(NULL, "/");
				val.out_of_port = atoi(out_of_port_str);
			}
			val.dst_mac[0] = (int)strtol( strtok(dst_mac, ":") , NULL, 16);
			val.dst_mac[1] = (int)strtol( strtok(NULL, ":") , NULL, 16);
			val.dst_mac[2] = (int)strtol( strtok(NULL, ":") , NULL, 16);

			val.dst_mac[3] = (int)strtol( strtok(NULL, ":") , NULL, 16);
			val.dst_mac[4] = (int)strtol( strtok(NULL, ":") , NULL, 16);
			val.dst_mac[5] = (int)strtol( strtok(NULL, ":") , NULL, 16);
		}

		switch (argv[2][0]){
			case 'i':
			case 'u':
				if(argc != 5) return -1;
				bpf_map_update_elem(map_fd, &key4, &val, 0);
				break;
			case 'd':
				bpf_map_delete_elem(map_fd, &key4);
				break;
			case 'l':
				bpf_map_lookup_elem(map_fd, &key4, &val);
				printf("val.dst_mac: %02x:%02x:%02x : %02x:%02x:%02x\n", val.dst_mac[0], val.dst_mac[1], val.dst_mac[2], val.dst_mac[3], val.dst_mac[4], val.dst_mac[5]);
				printf("val.out_of_port: %d\n", val.out_of_port);
				break;
		}
	}
	return 0;
}
