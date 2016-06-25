#ifndef __IFADDRS_H__
#define __IFADDRS_H__

#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <string.h>

struct ifaddrs
{
  struct ifaddrs *ifa_next;	/* Pointer to the next structure.  */
  char *ifa_name;		/* Name of this network interface.  */
  unsigned int ifa_flags;	/* Flags as from SIOCGIFFLAGS ioctl.  */
  struct sockaddr *ifa_addr;	/* Network address of this interface.  */
  struct sockaddr *ifa_netmask; /* Netmask of this interface.  */
  union
  {
    struct sockaddr *ifu_broadaddr; /* Broadcast address of this interface. */
    struct sockaddr *ifu_dstaddr; /* Point-to-point destination address.  */
  } ifa_ifu;
# ifndef ifa_broadaddr
#  define ifa_broadaddr	ifa_ifu.ifu_broadaddr
# endif
# ifndef ifa_dstaddr
#  define ifa_dstaddr	ifa_ifu.ifu_dstaddr
# endif
  void *ifa_data;
};

static int getifaddrs(struct ifaddrs **ifap)
{
	struct ifconf ifc;
	struct ifreq *ifr;
	struct ifaddrs *last_ifa = NULL, *ifa = NULL;
	int i, err = 0;
	int ifc_ctl_sock;

	ifc_ctl_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (ifc_ctl_sock < 0)
		return -1;

	ifc.ifc_len = 4096;
	ifc.ifc_buf = (char *)malloc(ifc.ifc_len);
	if (!ifc.ifc_buf) {
		close(ifc_ctl_sock);
		return -1;
	}
	memset(ifc.ifc_buf, 0, ifc.ifc_len);

	if (ifap)
		*ifap = NULL;

	if (ioctl(ifc_ctl_sock, SIOCGIFCONF, &ifc) < 0) {
		free(ifc.ifc_buf);
		close(ifc_ctl_sock);
		return -1;
	}

	for (i = 0; i < ifc.ifc_len/sizeof(struct ifreq); i++) {
		ifr = &ifc.ifc_req[i];

		if (ifr->ifr_name[0] == 0)
			continue;

		ifa = (struct ifaddrs *)malloc(sizeof(struct ifaddrs));
		if (!ifa) {
			err = -1;
			break;
		}

		memset(ifa, 0, sizeof(struct ifaddrs));

		if (ifap && (*ifap == NULL))
			*ifap = ifa;

		ifa->ifa_name = strdup(ifr->ifr_name);

        if(ioctl(ifc_ctl_sock, SIOCGIFADDR, ifr) < 0) {
            ifa->ifa_addr = 0;
        } else {
			ifa->ifa_addr = (struct sockaddr *)malloc(sizeof(struct sockaddr));
			if (ifa->ifa_addr)
				memcpy(ifa->ifa_addr, &ifr->ifr_addr, sizeof(struct sockaddr));
        }

		if (ioctl(ifc_ctl_sock, SIOCGIFNETMASK, ifr) < 0) {
			ifa->ifa_netmask = 0;
		} else {
			ifa->ifa_netmask = (struct sockaddr *)malloc(sizeof(struct sockaddr));
			if (ifa->ifa_netmask)
				memcpy(ifa->ifa_netmask, &ifr->ifr_netmask, sizeof(struct sockaddr));
		}

		if (ioctl(ifc_ctl_sock, SIOCGIFFLAGS, ifr) < 0) {
			ifa->ifa_flags = 0;
		} else {
			ifa->ifa_flags = ifr->ifr_flags;
		}

		if (ioctl(ifc_ctl_sock, SIOCGIFBRDADDR, ifr) < 0) {
			ifa->ifa_broadaddr = 0;
		} else {
			ifa->ifa_broadaddr = (struct sockaddr *)malloc(sizeof(struct sockaddr));
			if (ifa->ifa_broadaddr)
				memcpy(ifa->ifa_broadaddr, &ifr->ifr_broadaddr, sizeof(struct sockaddr));
		}

		if (last_ifa) {
			last_ifa->ifa_next = ifa;
		}

		last_ifa = ifa;
	}

	close(ifc_ctl_sock);
	return err;
}

static void freeifaddrs(struct ifaddrs *ifa)
{
    struct ifaddrs *ifp;

    while (ifa) {
        ifp = ifa;
		if (ifp->ifa_name)
        	free(ifp->ifa_name);
        if (ifp->ifa_addr)
            free(ifp->ifa_addr);
        if (ifp->ifa_netmask)
            free(ifp->ifa_netmask);
		if (ifp->ifa_broadaddr)
			free(ifp->ifa_broadaddr);
        ifa = ifa->ifa_next;
        free(ifp);
    }
}

#endif
