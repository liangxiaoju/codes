#!/usr/bin/python

import struct
import sys, os
from optparse import OptionParser

magic_number = "~Module signature appended~\n"

class Module:

    def read_data(self, offset, size):
        return self.__data[offset:offset+size]

    def write_data(self, offset, value):
        self.__data = self.__data[0:offset] + value + self.__data[offset+len(value):]

    def save_as(self, filename):
        with open(filename, "wb") as f:
            f.write(self.__data)

    def _next_string(self, offset, size):
        name = ""

        while True:
            c = self.read_data(offset, 1)
            if c != '\0':
                break
            offset += 1
            size -= 1
            if size <= 0:
                return None

        while True:
            c = self.read_data(offset, 1)
            offset += 1
            size -= 1
            if c == '\0':
                break
            else:
                name += c
            if size <= 0:
                return None
        return (name, offset, size)

    def __init__(self, filename):
        self.filename = filename
        with open(filename, "rb") as f:
            self.__data = f.read()

        self._parse_elfhdr()
        self._parse_sechdrs()

    def _parse_elfhdr(self):

        self.elfhdr = elfhdr = {}

        magic = [ord(i) for i in self.read_data(0, 16)]

        if magic[0] != 127 or magic[1] != ord('E') or magic[2] != ord('L') or magic[3] != ord('F'):
            raise Exception("not an elf file.")

        if magic[4] == 1:
            elfhdr_fmt = "=2H5I6H"
            sechdr_fmt = "=10I"
            symcrc_fmt = "=L" + str((64 - struct.calcsize("=L"))) + "s"
        else:
            elfhdr_fmt = "=2HI3QI6H"
            sechdr_fmt = "=2I4Q2I2Q"
            symcrc_fmt = "L" + str((64 - struct.calcsize("L"))) + "s"

        self.elfhdr_fmt = elfhdr_fmt
        self.sechdr_fmt = sechdr_fmt
        self.symcrc_fmt = symcrc_fmt

        temp = self.read_data(16, struct.calcsize(elfhdr_fmt))
        temp = struct.unpack(elfhdr_fmt, temp)

        elfhdr["magic"] = magic
        elfhdr["e_type"] = temp[0]
        elfhdr['e_machine'] = temp[1]
        elfhdr['e_version'] = temp[2]
        elfhdr['e_entry'] = temp[3]
        elfhdr['e_phoff'] = temp[4]
        elfhdr['e_shoff'] = temp[5]
        elfhdr['e_flags'] = temp[6]
        elfhdr['e_ehsize'] = temp[7]
        elfhdr['e_phentsize'] = temp[8]
        elfhdr['e_phnum'] = temp[9]
        elfhdr['e_shentsize'] = temp[10]
        elfhdr['e_shnum'] = temp[11]
        elfhdr['e_shstrndx'] = temp[12]

    def dump_elfhdr(self):
        print "\n# dump elf header:"

        elfhdr = self.elfhdr
        magic = elfhdr['magic']
        print "  Magic:", magic

        if magic[4] == 1:
            print "  Class: ELF32"
        else:
            print "  Class: ELF64"

        if magic[5] == 1:
            print "  Data: 2's complement,little endian"
        else:
            print "  Data: 2's complement,bigendian"

        print "  Version: %d(current)" %magic[6]

        if magic[7] == 0:
            os_abi = 'System V ABI'
        elif magic[7]== 1:
            os_abi = 'HP-Ux operating system'
        elif magic[7] == 255:
            os_abi = 'Standalone (embedded) application'
        print "  OS/ABI: %s" %os_abi

        print "  ABI Version: %d" %magic[8]

        if elfhdr['e_type'] == 0:
            type = 'No file type'
        elif elfhdr['e_type'] == 1:
            type = 'Relocatable object file'
        elif elfhdr['e_type'] == 2:
            type = 'Executable file'
        elif elfhdr['e_type'] == 3:
            type = 'Core file'
        print "  Type: %s" %type

        print "  Machine: %d" %elfhdr['e_machine']
        print "  Version: 0x%x" %elfhdr['e_version']
        print "  Entry point address: 0x%x" %elfhdr['e_entry']
        print "  Start of program headers: %d (bytes into file)" %elfhdr['e_phoff']
        print "  Start of section headers: %d (bytes into file)" %elfhdr['e_shoff']
        print "  Flags: 0x%x" %elfhdr['e_flags']
        print "  Size of this header: %d (bytes)" %elfhdr['e_ehsize']
        print "  Size of program headers: %d (bytes)" %elfhdr['e_phentsize']
        print "  Number of program headers: %d " %elfhdr['e_phnum']
        print "  Size of section headers: %d (bytes)" %elfhdr['e_shentsize']
        print "  Number of section headers: %d" %elfhdr['e_shnum']
        print "  Section header string table index: %d"%elfhdr['e_shstrndx']

    def _parse_sechdrs(self):
        elfhdr = self.elfhdr
        sechdrs = elfhdr["e_shoff"]
        secsize = elfhdr["e_shentsize"]
        secnums = elfhdr["e_shnum"]
        secs = []

        for i in range(0, secnums):
            temp = self.read_data(sechdrs+i*secsize, secsize)
            temp = struct.unpack(self.sechdr_fmt, temp)
            sec = {}
            sec['sh_name'] = temp[0]
            sec['sh_type'] = temp[1]
            sec['sh_flags'] = temp[2]
            sec['sh_addr'] = temp[3]
            sec['sh_offset'] = temp[4]
            sec['sh_size'] = temp[5]
            sec['sh_link'] = temp[6]
            sec['sh_info'] = temp[7]
            sec['sh_addralign'] = temp[8]
            sec['sh_entsize'] = temp[9]
            secs.append(sec)

        secstrings = secs[elfhdr['e_shstrndx']]['sh_offset']

        names = []
        for i in range(0, secnums):
            offset = secstrings + secs[i]['sh_name']
            names.append(self._next_string(offset, 256)[0])

        self.sechdrs = dict(zip(names, secs))
        self.secname = names

    def dump_sechdrs(self):
        print "\n# dump sections header:"

        print "  [NR]\tname\taddr\toffset\tsize\tentsize"
        for i in range(0, len(self.sechdrs)):
            name = self.secname[i]
            sec = self.sechdrs[name]
            print "  %d\t%s\t%s\t%s\t%s\t%s" % \
                    (i, name, sec['sh_addr'], sec['sh_offset'], sec['sh_size'], sec['sh_entsize'])

    def get_section(self, secname):
        sec = self.sechdrs[secname]
        addr = sec['sh_addr'] + sec['sh_offset']
        size = sec['sh_size']
        return (sec, addr, size)

    def get_modinfo(self, tag):
        sec, addr, size = self.get_section(".modinfo")
        while size > 0:
            pack = self._next_string(addr, size)
            if pack == None:
                break
            sym, addr, size = pack
            t, v = sym.split("=", 1)
            if  t == tag:
                return v

    def set_modinfo(self, tag, value):
        sec, addr, size = self.get_section(".modinfo")
        while size > 0:
            pack = self._next_string(addr, size)
            if pack == None:
                break
            sym, addr, size = pack
            t, v = sym.split("=", 1)
            if  t == tag:
                print "%s --> %s" % (v, value)
                self.write_data(addr-1-len(v), value+'\0'*(len(v)-len(value)))

    def dump_modinfo(self, tag=None):
        print "\n# dump .modinfo symbols:"

        sec, addr, size = self.get_section(".modinfo")
        while size > 0:
            pack = self._next_string(addr, size)
            if pack == None:
                break
            sym, addr, size = pack
            t, v = sym.split("=", 1)
            if tag == None or t == tag:
                print "  " + sym

    def set_symvers(self, symbol, crc):
        sec, addr, size = self.get_section("__versions")

        fmt = self.symcrc_fmt
        unit = struct.calcsize(fmt)
        num = size / unit

        for i in range(0, num):
            temp = struct.unpack(fmt, self.read_data(addr+i*unit, unit))
            if temp[1] == symbol.ljust(len(temp[1]), "\0"):
                if crc != temp[0]:
                    data = struct.pack(fmt, crc, temp[1])
                    self.write_data(addr+i*unit, data)
                    print "0x%08lx --> 0x%08lx %s" % (temp[0], crc, symbol)

    def dump_symvers(self, symbol=None):
        print "\n# dump symbol crc:"

        sec, addr, size = self.get_section("__versions")

        fmt = self.symcrc_fmt
        unit = struct.calcsize(fmt)
        num = size / unit

        for i in range(0, num):
            temp = struct.unpack(fmt, self.read_data(addr+i*unit, unit))
            if symbol == None or temp[1] == symbol.ljust(len(temp[1]), "\0"):
                print "  0x%08lx %s" % (temp[0], temp[1])

    def unsign(self):
        temp = self.__data[-len(magic_number):]
        if temp == magic_number:
            offset = -(len(magic_number) + struct.calcsize(">BBBBBxxxI"))
            pack = struct.unpack(">BBBBBxxxI", self.__data[offset:offset+struct.calcsize(">BBBBBxxxI")])
            (algo, hash, id_type, signer_len, key_id_len, sig_len) = pack
            offset -= (sig_len + key_id_len + signer_len)
            self.__data = self.__data[:offset]

    def sign(self, dgst, key, cert):
        try:
            from M2Crypto import X509, EVP, RSA
        except:
            print "\nYou need to install M2Crypto module !"
            print "Please run 'apt-get install python-m2crypto'\n"
            sys.exit(-1)

        # try unsign first
        self.unsign()

        x509 = X509.load_cert(cert, 0)
        x509_name = x509.get_subject()

        key_identifier = x509.get_ext("subjectKeyIdentifier").get_value()
        org = getattr(x509_name, "organizationName")
        cn = getattr(x509_name, "commonName")
        email = getattr(x509_name, "emailAddress")

        if not key_identifier:
            raise Exception("Couldn't find subjectKeyIdentifier.")
        temp = key_identifier.split(":")
        temp = [ int(i, 16) for i in temp ]
        key_identifier = ""
        for i in temp:
            key_identifier += struct.pack("B", i)

        if org and cn:
            if len(org) <= len(cn) and cn[:len(org)] == org:
                signers_name = cn
            elif len(org) >= 7 and len(cn) >= 7 and cn[:7] == org[:7]:
                signers_name = cn
            else:
                signers_name = org + ": " + cn
        elif org:
            signers_name = org
        elif cn:
            signers_name = cn
        else:
            signers_name = email

        md = EVP.MessageDigest(dgst)
        md.update(self.__data)
        digest = md.final()
        digest_tmp = struct.unpack(len(digest)*"B", digest)

        rsa = RSA.load_key(key)
        signature = rsa.sign(digest, dgst)
        signature = struct.pack(">H", len(signature)) + signature
        signature_tmp = struct.unpack(len(signature)*"B", signature)

        algo = 1
        hash_dict = { "sha1":2, "sha224":7, "sha256":4, "sha384":5, "sha512":6 }
        hash = hash_dict[dgst]
        id_type = 1

        info = struct.pack(">BBBBBxxxI", algo, hash, id_type,
                len(signers_name), len(key_identifier), len(signature))

        print "Private key            : %s" % key
        print "Public key             : %s" % cert
        print "Size of unsigned module: " + str(len(self.__data))
        print "Size of signer's name  : " + str(len(signers_name))
        print "Size of key identifier : " + str(len(key_identifier))
        print "Size of signature      : " + str(len(signature))
        print "Size of informaton     : " + str(len(info))
        print "Size of magic number   : " + str(len(magic_number))
        print "Signer's name          : " + "'" + signers_name + "'"
        print "organizationName       : " + org
        print "commonName             : " + cn
        print "emailAddress           : " + email
        print "Digest                 : " + dgst

        self.__data += signers_name + key_identifier + signature + info + magic_number


def try_trans_file(name):
    '''
        if the file no exist, try to find it in KERNEL_OBJ directory.
    '''
    name = os.path.expanduser(name)
    if os.path.isfile(name):
        return name

    pwd = os.path.realpath(__file__)
    pwd = os.path.dirname(pwd)
    path = os.path.join(pwd, "../../out/target/product")
    path = os.path.realpath(path)
    if os.path.isdir(path):
        for d in os.listdir(path):
            kobj = os.path.join(path, d, "obj/KERNEL_OBJ", name)
            kobj = os.path.realpath(kobj)
            if os.path.isfile(kobj):
                return kobj
    return name

def main():
    parser = OptionParser()
    parser.add_option("-i", dest="ifile", metavar="ifile.ko",
            help="the module input")
    parser.add_option("-o", dest="ofile", metavar="ofile.ko",
            help="the module output")
    parser.add_option("--sign", action="store_true", dest="sign",
            help="sign the module")
    parser.add_option("--unsign", action="store_true", dest="unsign",
            help="unsign the module")
    parser.add_option("--dgst", dest="dgst", default="sha512", metavar="digest",
            help="digest algo sha1/sha256/sha512 (default: sha512)")
    parser.add_option("--key", dest="key", default="./signing_key.priv",
            help="private key (default: ./signing_key.priv)")
    parser.add_option("--x509", dest="x509", default="./signing_key.x509",
            help="x509 key (default: ./signing_key.x509)")
    parser.add_option("--vermagic", dest="vermagic", metavar="string",
            help="set version magic string")
    parser.add_option("--symvers", dest="symvers", metavar="file",
            help="set symbol's crc according to Module.symvers")
    parser.add_option("-d", "--dump", action="store_true", dest="dump",
            help="dump simple info of module")
    parser.add_option("-a", "--all", action="store_true", dest="dumpall",
            help="dump all info of module")

    (options, args) = parser.parse_args()

    ifile = options.ifile
    if ifile == None:
        parser.print_help()
        sys.exit(-1)

    ofile = options.ofile

    vermagic = options.vermagic
    symvers = options.symvers

    module = Module(ifile)

    if options.dumpall:
        module.dump_elfhdr()
        module.dump_sechdrs()
        module.dump_modinfo()
        module.dump_symvers()
    elif options.dump:
        module.dump_modinfo()
        module.dump_symvers("module_layout")

    if vermagic != None:
        module.set_modinfo("vermagic", str(vermagic))

    if symvers != None:
        if symvers == "":
            symvers = "./Module.symvers"

        symvers = try_trans_file(symvers)

        if symvers and os.path.isfile(symvers):
            print "Module.symvers         : %s" % symvers
            with open(symvers) as f:
                while True:
                    line = f.readline()
                    if not line: break
                    line = line.split()
                    module.set_symvers(line[1], int(line[0], 16))
        elif symvers:
            pairs = symvers.split(",")
            for pair in pairs:
                (symbol, crc) = pair.split(":", 1)
                module.set_symvers(symbol, int(crc, 16))

    if options.unsign:
        module.unsign()

    if options.sign:
        key = try_trans_file(options.key)
        x509 = try_trans_file(options.x509)

        if not (os.path.isfile(key) and os.path.isfile(x509)):
            print "no private key found!"
            print "no x509 pub key found!"
            sys.exit(-1)

        module.sign(options.dgst, key, x509)

    if ofile:
        module.save_as(ofile)


if __name__ == "__main__":
    main()

