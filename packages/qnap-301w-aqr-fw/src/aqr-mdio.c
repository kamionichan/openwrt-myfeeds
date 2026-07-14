#include <errno.h>
#include <linux/mii.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#define MDIO_PHY_ID_C45 0x8000
#define MDIO_PHY_ID_PRTAD 0x03e0
#define MDIO_PHY_ID_DEVAD 0x001f
#define SSDK_C45_MARK (1U << 30)

static unsigned int phy_id_c45(unsigned int phy, unsigned int mmd)
{
    return MDIO_PHY_ID_C45 |
           ((phy << 5) & MDIO_PHY_ID_PRTAD) |
           (mmd & MDIO_PHY_ID_DEVAD);
}

static int mdio_ioctl(int fd, const char *ifname, unsigned long cmd,
                      unsigned int phy, unsigned int mmd, unsigned int reg,
                      uint16_t *value)
{
    struct ifreq ifr;
    struct mii_ioctl_data mii;

    memset(&ifr, 0, sizeof(ifr));
    memset(&mii, 0, sizeof(mii));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", ifname);
    mii.phy_id = phy_id_c45(phy, mmd);
    mii.reg_num = reg;
    if (cmd == SIOCSMIIREG)
        mii.val_in = *value;
    memcpy(&ifr.ifr_data, &mii, sizeof(mii));

    if (ioctl(fd, cmd, &ifr) < 0)
        return -1;

    if (cmd == SIOCGMIIREG) {
        memcpy(&mii, &ifr.ifr_data, sizeof(mii));
        *value = mii.val_out;
    }
    return 0;
}

static int parse_u32(const char *s, unsigned long max, unsigned int *out)
{
    char *end;
    unsigned long v;

    errno = 0;
    v = strtoul(s, &end, 0);
    if (end == s || errno || *end != '\0' || v > max)
        return -1;
    *out = (unsigned int)v;
    return 0;
}

static int do_read(int fd, const char *ifname, unsigned int phy,
                   unsigned int mmd, unsigned int reg, uint16_t *value)
{
    if (mdio_ioctl(fd, ifname, SIOCGMIIREG, phy, mmd, reg, value) < 0) {
        fprintf(stderr, "read %s PHY 0x%x MMD 0x%x reg 0x%04x: %s\n",
                ifname, phy, mmd, reg, strerror(errno));
        return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    const char *op, *ifname;
    unsigned int phy, mmd, reg, val_u32;
    uint16_t value;
    int fd, rc = 0;

    if (argc < 2) {
        fprintf(stderr,
                "Usage:\n"
                "  %s read NETDEV PHY MMD REG\n"
                "  %s write NETDEV PHY MMD REG VALUE\n"
                "  %s fw-id NETDEV PHY\n"
                "  %s ssdk-reg PHY MMD REG\n",
                argv[0], argv[0], argv[0], argv[0]);
        return 64;
    }

    op = argv[1];
    if (!strcmp(op, "ssdk-reg")) {
        if (argc != 5 || parse_u32(argv[2], 31, &phy) ||
            parse_u32(argv[3], 31, &mmd) ||
            parse_u32(argv[4], 0xffff, &reg)) {
            fprintf(stderr, "Usage: %s ssdk-reg PHY MMD REG\n", argv[0]);
            return 64;
        }
        printf("0x%08x\n", SSDK_C45_MARK | (mmd << 16) | reg);
        return 0;
    }

    if (!strcmp(op, "fw-id")) {
        uint16_t fw_id, stat1, misc_id, misc_ver;
        if (argc != 4 || parse_u32(argv[3], 31, &phy)) {
            fprintf(stderr, "Usage: %s fw-id NETDEV PHY\n", argv[0]);
            return 64;
        }
        ifname = argv[2];
        fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0) {
            perror("socket");
            return 70;
        }
        rc = do_read(fd, ifname, phy, 30, 0x0020, &fw_id) ||
             do_read(fd, ifname, phy, 30, 0xc885, &stat1) ||
             do_read(fd, ifname, phy, 1, 0xc41d, &misc_id) ||
             do_read(fd, ifname, phy, 1, 0xc41e, &misc_ver);
        close(fd);
        if (rc)
            return 74;
        printf("netdev=%s phy=0x%02x\n", ifname, phy);
        printf("FW_ID=0x%04x version=%u.%u\n", fw_id,
               (fw_id >> 8) & 0xff, fw_id & 0xff);
        printf("RSVD_STAT1=0x%04x build=%u provisioning=%u\n", stat1,
               (stat1 >> 4) & 0x0f, stat1 & 0x0f);
        printf("MISC_ID=0x%04x decimal=%u\n", misc_id, misc_id);
        printf("MISC_VER=0x%04x decimal=%u\n", misc_ver, misc_ver);
        return 0;
    }

    if ((!strcmp(op, "read") && argc != 6) ||
        (!strcmp(op, "write") && argc != 7) ||
        (strcmp(op, "read") && strcmp(op, "write"))) {
        fprintf(stderr, "Invalid operation or argument count\n");
        return 64;
    }

    ifname = argv[2];
    if (parse_u32(argv[3], 31, &phy) || parse_u32(argv[4], 31, &mmd) ||
        parse_u32(argv[5], 0xffff, &reg)) {
        fprintf(stderr, "Invalid PHY/MMD/register\n");
        return 64;
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        return 70;
    }

    if (!strcmp(op, "read")) {
        if (do_read(fd, ifname, phy, mmd, reg, &value) < 0)
            rc = 74;
        else
            printf("0x%04x\n", value);
    } else {
        if (parse_u32(argv[6], 0xffff, &val_u32)) {
            fprintf(stderr, "Invalid value\n");
            close(fd);
            return 64;
        }
        value = (uint16_t)val_u32;
        if (mdio_ioctl(fd, ifname, SIOCSMIIREG, phy, mmd, reg, &value) < 0) {
            fprintf(stderr, "write %s PHY 0x%x MMD 0x%x reg 0x%04x: %s\n",
                    ifname, phy, mmd, reg, strerror(errno));
            rc = 74;
        }
    }
    close(fd);
    return rc;
}
