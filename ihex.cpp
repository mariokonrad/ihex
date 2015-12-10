// ihex.cpp
// (c) 2015 Mario Konrad <mario.konrad@gmx.net>
//
// this software is distributed under the license: GPLv2 (http://www.gnu.org/licenses/gpl-2.0.html)

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <set>
#include <memory>
#include <stdint.h>
#include <getopt.h>

class not_implemented : public std::exception
{
	public:
		const char * file;
		int line;
		std::string text;
	private:
		void update(void)
		{
			std::ostringstream os;
			os << "NOT IMPLEMENTED: " << file << ":" << line;
			text = os.str();
		}
	public:
		not_implemented(const char * file, int line)
			: file(file)
			, line(line)
		{
			update();
		}

		virtual ~not_implemented() throw () {}

		virtual const char * what() const throw()
		{
			return text.c_str();
		}
};

inline uint8_t h2b(char h)
{
	if (h >= '0' && h <= '9') return h-'0';
	if (h >= 'A' && h <= 'F') return h-'A'+10;
	if (h >= 'a' && h <= 'f') return h-'a'+10;
	return 0x00;
}

std::istringstream & operator >> (std::istringstream & is, uint8_t & val)
{
	char t;

	val = 0;
	is >> t;
	val += h2b(t);
	val <<= 4;
	is >> t;
	val += h2b(t);
	return is;
}

std::istringstream & operator >> (std::istringstream & is, uint16_t & val)
{
	uint8_t t;

	val = 0;
	is >> t;
	val += t;
	val <<= 8;
	is >> t;
	val += t;
	return is;
}

class Record
{
	public:
		enum Type : uint8_t {
			 DATA              = 0x00
			,END_OF_FILE       = 0x01
			,EXT_SEG_ADDRESS   = 0x02
			,START_SEG_ADDRESS = 0x03
			,EXT_LIN_ADDRESS   = 0x04
			,START_LIN_ADDRESS = 0x05
		};

		typedef uint8_t value_type;
		typedef std::vector<value_type> Data;
		typedef Data::size_type size_type;
		typedef uint8_t checksum_type;
		typedef uint16_t offset_type;
		typedef uint32_t address_type;
		typedef Data::const_iterator const_iterator;

		class checksum_exception : public std::exception
		{
			public:
				checksum_type checksum;
				checksum_type calculated;
				int line;
			public:
				checksum_exception(checksum_type checksum, checksum_type calculated)
					: checksum(checksum)
					, calculated(calculated)
					, line(-1)
				{}

				checksum_exception(const checksum_exception & ex, int line)
					: checksum(ex.checksum)
					, calculated(ex.calculated)
					, line(line)
				{}
		};

		class unknown_type_exception : public std::exception
		{
			public:
				Type type;
			public:
				unknown_type_exception(Type type)
					: type(type)
				{}
		};
	private:
		offset_type off;
		Type t;
		Data bytes;
	public:
		Record(void);
		explicit Record(Type);
		explicit Record(address_type);

		void push_back(value_type);

		Type type(void) const;
		checksum_type checksum(void) const;
		size_type size(void) const;
		offset_type offset(void) const;

		const_iterator begin(void) const;
		const_iterator end(void) const;

		address_type address(void) const;

		static Record create_data(address_type);
		static Record eof(void);

		friend std::istream & operator >> (std::istream &, Record &) throw (checksum_exception, unknown_type_exception);
		friend std::ostream & operator << (std::ostream &, const Record &);
};

Record Record::create_data(address_type address)
{
	Record rec(Type::DATA);
	rec.off = address & 0xffff;
	return rec;
}

Record Record::eof(void)
{
	return Record(Type::END_OF_FILE);
}

Record::Record(void)
	: off(0)
	, t(Type::END_OF_FILE)
{}

Record::Record(Type type)
	: off(0)
	, t(type)
{}

Record::Record(address_type address)
	: off(0)
	, t(Type::EXT_LIN_ADDRESS)
{
	bytes.reserve(2);
	bytes.push_back((address >> 24) & 0xff);
	bytes.push_back((address >> 16) & 0xff);
}

Record::address_type Record::address(void) const
{
	if (bytes.size() < 2) return 0;
	address_type address = 0;
	address += bytes[0];
	address <<= 8;
	address += bytes[1];
	address <<= 16;
	return address;
}

Record::const_iterator Record::begin(void) const
{
	return bytes.begin();
}

Record::const_iterator Record::end(void) const
{
	return bytes.end();
}

Record::size_type Record::size(void) const
{
	return bytes.size();
}

Record::Type Record::type(void) const
{
	return t;
}

Record::offset_type Record::offset(void) const
{
	return off;
}

void Record::push_back(value_type val)
{
	bytes.push_back(val);
}

Record::checksum_type Record::checksum(void) const
{
	checksum_type sum = 0;
	sum += static_cast<checksum_type>(bytes.size());
	sum += (off >> 8) & 0xff;
	sum += (off >> 0) & 0xff;
	sum += static_cast<uint16_t>(type());
	for (auto i = bytes.begin(); i != bytes.end(); ++i) {
		sum += *i;
	}
	return -sum;
}

std::istream & operator >> (std::istream & is, Record & rec)
		throw (Record::checksum_exception, Record::unknown_type_exception)
{
	using namespace std;

	uint8_t len;
	uint8_t type;
	Record::checksum_type checksum;

	string line;
	getline(is, line);
	istringstream iss(line);

	char mark;
	iss >> mark; // consume leading colon ':'

	iss >> len;
	iss >> rec.off;
	iss >> type;

	Record::Type rtype = static_cast<Record::Type>(type);
	switch (rtype) {
		case Record::Type::END_OF_FILE:
		case Record::Type::EXT_LIN_ADDRESS:
		case Record::Type::DATA:
			break;
		case Record::Type::EXT_SEG_ADDRESS:
		case Record::Type::START_SEG_ADDRESS:
		case Record::Type::START_LIN_ADDRESS:
			throw not_implemented(__FILE__, __LINE__);
		default:
			throw Record::unknown_type_exception(rtype);
	}

	rec.t = rtype;

	rec.bytes.clear();
	rec.bytes.reserve(len);
	for (uint8_t i = 0; i < len; ++i) {
		Record::value_type b;
		iss >> b;
		rec.bytes.push_back(b);
	}
	iss >> checksum;

	if (checksum != rec.checksum()) throw Record::checksum_exception(checksum, rec.checksum());

	return is;
}

std::ostream & operator << (std::ostream & os, const Record & rec)
{
	using namespace std;

	os	<< ":"
		<< setbase(16)
		<< uppercase
		<< setfill('0')
		<< setw(2)
		<< rec.size()
		<< setw(4)
		<< rec.offset()
		<< setw(2)
		<< rec.type()
		;
	for (auto i = rec.bytes.begin(); i != rec.bytes.end(); ++i) {
		os	<< setbase(16)
			<< uppercase
			<< setfill('0')
			<< setw(2)
			<< static_cast<int>(*i)
			;
	}
	os	<< setbase(16)
		<< uppercase
		<< setfill('0')
		<< setw(2)
		<< static_cast<int>(rec.checksum())
		<< endl
		;
	return os;
}

class Region
{
	public:
		class continuous_exception : public std::exception {};
		typedef Record::address_type address_type;
	private:
		typedef Record::value_type value_type;
		typedef std::vector<value_type> Data;
		typedef Record::offset_type offset_type;
	public:
		typedef Data::size_type size_type;
	private:
		Data data; // max size 64kB, since offset of hex file is 16 bits
		address_type base_address;
		offset_type offset;
		bool offset_already_set;
	public:
		Region(address_type = 0);
		void insert(offset_type, value_type) throw (continuous_exception);
		size_type size(void) const;
		void dump_data(std::ostream &, unsigned int = 16) const;
		void dump_ihex(std::ostream &, unsigned int = 32) const;
		address_type address(void) const;
		void move_base_address(address_type);
		bool inside(address_type) const;
};

Region::Region(address_type base_address)
	: base_address(base_address)
	, offset(0)
	, offset_already_set(false)
{}

Region::address_type Region::address(void) const
{
	return base_address + offset;
}

void Region::move_base_address(address_type destination_base_address)
{
	base_address = destination_base_address & 0xffff0000;
	offset = destination_base_address & 0x0000ffff;
}

bool Region::inside(address_type address) const
{
	return true
		&& (address >= base_address + offset)
		&& (address < base_address + offset - size())
		;
}

void Region::insert(offset_type value_offset, value_type value) throw (continuous_exception)
{
	if (offset_already_set) {
		if (value_offset < offset) throw continuous_exception();
		if (value_offset > data.size() + offset) throw continuous_exception();
	} else {
		offset = value_offset;
		offset_already_set = true;
	}
	data.push_back(value);
}

Region::size_type Region::size(void) const
{
	return data.size();
}

void Region::dump_data(std::ostream & os, unsigned int width) const
{
	size_type count = 0;
	address_type address = base_address + offset;

	os << std::setbase(16) << std::resetiosflags(std::ios::showbase);
	for (auto i = data.begin(); i != data.end(); ++i, ++address) {
		if (count >= width) {
			count = 0;
			os << std::endl;
		}
		if (count == 0) {
			os << "0x" << std::setfill('0') << std::setw(8) << address << " :";
		}
		os << " " << std::setfill('0') << std::setw(2) << static_cast<int>(*i);
		++count;
	}
	os << std::endl;
}

void Region::dump_ihex(std::ostream & os, unsigned int width) const
{
	enum class State {
		 NEW_RECORD
		,DATA
	};

	address_type address = base_address;
	State state = State::NEW_RECORD;
	Record rec;

	os << Record(address);
	for (auto i = data.begin(); i != data.end();) {
		switch (state) {
			case State::NEW_RECORD:
				rec = Record::create_data(address + offset);
				state = State::DATA;
				break;

			case State::DATA:
				if (rec.size() >= width) {
					os << rec;
					state = State::NEW_RECORD;
					break;
				}
				rec.push_back(*i);
				++address;
				++i;
				if (i == data.end()) {
					os << rec;
				}
				break;
		}
	}
}

class HexData
{
	private:
		typedef std::vector<Region> Data;
	public:
		typedef Data::const_iterator const_iterator;
		typedef Data::iterator iterator;
	private:
		Data data;
	public:
		HexData();
		void read_records(std::istream & is) throw (Record::checksum_exception);
		void dump_data(std::ostream &, unsigned int = 16) const;
		void dump_ihex(std::ostream &, unsigned int = 32) const;

		const_iterator find(Region::address_type) const;
		const_iterator begin(void) const;
		const_iterator end(void) const;

		iterator find(Region::address_type);
		iterator begin(void);
		iterator end(void);
		void erase(iterator);
};

HexData::HexData()
{}

HexData::const_iterator HexData::begin(void) const
{
	return data.begin();
}

HexData::const_iterator HexData::end(void) const
{
	return data.end();
}

HexData::iterator HexData::begin(void)
{
	return data.begin();
}

HexData::iterator HexData::end(void)
{
	return data.end();
}

HexData::const_iterator HexData::find(Region::address_type address) const
{
	for (auto i = data.begin(); i != data.end(); ++i) {
		if (i->address() == address) return i;
	}
	return end();
}

HexData::iterator HexData::find(Region::address_type address)
{
	for (auto i = data.begin(); i != data.end(); ++i) {
		if (i->address() == address) return i;
	}
	return end();
}

void HexData::erase(iterator i)
{
	data.erase(i);
}

void HexData::dump_data(std::ostream & os, unsigned int width) const
{
	for (auto i = data.begin(); i != data.end(); ++i) {
		i->dump_data(os, width);
	}
}

void HexData::dump_ihex(std::ostream & os, unsigned int width) const
{
	for (auto i = data.begin(); i != data.end(); ++i) {
		i->dump_ihex(os, width);
	}
	os << Record::eof();
}

void HexData::read_records(std::istream & is) throw (Record::checksum_exception)
{
	Region region;

	int line = 0;
	while (!is.eof()) {
		++line;

		Record rec;

		try {
			is >> rec;
		} catch (Record::checksum_exception e) {
			throw Record::checksum_exception(e, line);
		}

		switch (rec.type()) {
			case Record::Type::DATA:
				for (auto i = rec.begin(); i != rec.end(); ++i) {
					region.insert(rec.offset(), *i);
				}
				break;

			case Record::Type::END_OF_FILE:
				if (region.size()) {
					data.push_back(region);
				}
				return;

			case Record::Type::EXT_LIN_ADDRESS:
				if (region.size()) {
					data.push_back(region);
				}
				region = Region(rec.address());
				break;

			default:
				break;
		}
	}
}

static void print_info(std::ostream & os, const HexData & hex)
{
	using namespace std;

	Region::address_type total_size = 0;

	for (auto region = hex.begin(); region != hex.end(); ++region) {
		os	<< "0x" << setbase(16) << setfill('0') << setw(8) << region->address()
			<< "-"
			<< "0x" << setbase(16) << setfill('0') << setw(8) << region->address() + region->size() - 1
			<< " "
			<< "0x" << setbase(16) << setfill('0') << setw(4) << region->size()
			<< setbase(10) << resetiosflags(ios::showbase)
			<< endl;
		total_size += region->size();
	}
	os	<< endl
		<< "total size: " << total_size << " bytes"
		<< endl;
}

static struct Options {
	bool help;
	bool version;
	bool info;
	bool dump;
	bool ihex;
	unsigned int dump_width;
	unsigned int ihex_width;
	std::string input_filename;
	std::string output_filename;
	std::set<Region::address_type> erase_region;
	std::set<std::pair<Region::address_type, Region::address_type>> move_region;
} options = { false, false, false, false, false, 16, 32, "", "", {}, {} };

enum Option : int {
	 HELP = 0
	,DUMP
	,IHEX
	,INPUT
	,OUTPUT
	,ERASE_REGION
	,INFO
	,MOVE_REGION
	,VERSION
};

static const struct option LONG_OPTIONS[] =
{
	{ "help",         no_argument,       NULL, Option::HELP         },
	{ "dump",         optional_argument, NULL, Option::DUMP         },
	{ "ihex",         optional_argument, NULL, Option::IHEX         },
	{ "input",        required_argument, NULL, Option::INPUT        },
	{ "output",       required_argument, NULL, Option::OUTPUT       },
	{ "erase-region", required_argument, NULL, Option::ERASE_REGION },
	{ "info",         no_argument,       NULL, Option::INFO         },
	{ "move-region",  required_argument, NULL, Option::MOVE_REGION  },
	{ "version",      no_argument,       NULL, Option::VERSION      },
};

static void print_version(void)
{
	using namespace std;

	cout << "ihex 1.0.0" << endl;
	cout << endl;
	cout << "(c) 2015 Mario Konrad" << endl;
	cout << endl;
	cout << "This software is distributed under the terms of GPLv2." << endl;
	cout << "http://www.gnu.org/licenses/gpl-2.0.html" << endl;
	cout << endl;
	cout << "find the source at: http://www.mario-konrad.ch/wiki/doku.php?id=software:ihex:start" << endl;
	cout << "or at github: https://github.com/mariokonrad/ihex" << endl;
}

static void usage(const char * name)
{
	using namespace std;

	cout << endl;
	print_version();
	cout << endl;
	cout << "usage: " << name << " [options]" << endl;
	cout << endl;
	cout << "Options:" << endl;
	cout << "\t" << "--help                        : this help information" << endl;
	cout << "\t" << "--version                     : prints the version of the program" << endl;
	cout << "\t" << "--info                        : shows general information about the hex file" << endl;
	cout << "\t" << "--input filename              : input file name, intel hex 8bit format" << endl;
	cout << "\t" << "--output filename             : output file name" << endl;
	cout << "\t" << "--dump [=width]               : output file as hex dump, width of the" << endl;
	cout << "\t" << "                                output [4..64], default:16" << endl;
	cout << "\t" << "--ihex [=width]               : output file as intel 8bit hex file, width of" << endl;
	cout << "\t" << "                                the output [8..64], default:32" << endl;
	cout << "\t" << "--erase-region address        : erases the specified region, address in hex" << endl;
	cout << "\t" << "                                this parameter may be specified multiple times" << endl;
	cout << "\t" << "--move-region address-address : moves entire regions, source address must exist," << endl;
	cout << "\t" << "                                target address must not be occupied" << endl;
	cout << "\t" << "                                this parameter may be specified multiple times" << endl;
	cout << "\t" << "                                specifying overlapping moves result in an undefined behaviour" << endl;
	cout << "\t" << "                                NOTE: not all overlapping/overwriting possibilities" << endl;
	cout << "\t" << "                                are being checked, be careful!" << endl;
	cout << endl;
}

static void erase_region_append(const char * optarg)
{
	Region::address_type address;

	std::istringstream(optarg) >> std::hex >> address;
	options.erase_region.insert(address);
}

static void move_region_append(const char * optarg)
{
	Region::address_type src;
	Region::address_type dst;
	char minus;

	std::istringstream(optarg) >> std::hex >> src >> minus >> dst;
	options.move_region.insert(
		std::pair<Region::address_type, Region::address_type>(src, dst));
}

static int parse_options(int argc, char ** argv)
{
	while (optind < argc) {
		int index = -1;
		int result = getopt_long(argc, argv, "", LONG_OPTIONS, &index);

		if (result < 0) return -1;
		switch (result) {
			case Option::HELP:
				options.help = true;
				return 0;

			case Option::DUMP:
				options.dump = true;
				if (optarg) {
					std::istringstream(optarg) >> options.dump_width;
					if (options.dump_width > 64) options.dump_width = 64;
					if (options.dump_width <  4) options.dump_width =  4;
				}
				break;

			case Option::IHEX:
				options.ihex = true;
				if (optarg) {
					std::istringstream(optarg) >> options.ihex_width;
					if (options.ihex_width > 64) options.ihex_width = 64;
					if (options.ihex_width <  8) options.ihex_width =  8;
				}
				break;

			case Option::INPUT:
				options.input_filename = optarg;
				break;

			case Option::OUTPUT:
				options.output_filename = optarg;
				break;

			case Option::ERASE_REGION:
				erase_region_append(optarg);
				break;

			case Option::INFO:
				options.info = true;
				break;

			case Option::MOVE_REGION:
				move_region_append(optarg);
				break;

			case Option::VERSION:
				options.version = true;
				break;

			default:
				return -1;
		}
	}
	if (optind < argc) return -1;
	return 0;
}

int main(int argc, char ** argv)
{
	using namespace std;

	// check command line parameters

	if (argc < 2) {
		usage(argv[0]);
		return -1;
	}
	int rc = parse_options(argc, argv);
	if (rc) return -1;

	if (options.version) {
		print_version();
		return 0;
	}

	if (options.help) {
		usage(argv[0]);
		return 0;
	}

	// handle input

	ifstream ifs;
	streambuf * cin_old = cin.rdbuf();
	if (options.input_filename.size()) {
		ifs.open(options.input_filename.c_str(), ios::in);
		if (!ifs) {
			cerr << "Error: cannot open input file: " << options.input_filename << endl;
			return -2;
		}
		cin.rdbuf(ifs.rdbuf());
	}

	// handle output

	ofstream ofs;
	streambuf * cout_old = cout.rdbuf();
	if (options.output_filename.size()) {
		ofs.open(options.output_filename.c_str(), ios::out);
		if (!ofs) {
			cerr << "Error: cannot open output file: " << options.output_filename << endl;
			return -2;
		}
		cout.rdbuf(ofs.rdbuf());
	}

	// read data

	HexData hex;

	try {
		hex.read_records(cin);
	} catch (Record::checksum_exception e) {
		cerr
			<< setbase(10) << resetiosflags(ios::showbase)
			<< "ERROR: " << argv[0] << ": record checksum error on line " << e.line << " : "
			<< setbase(16) << resetiosflags(ios::showbase)
			<< "0x" << setfill('0') << setw(2) << static_cast<int>(e.checksum)
			<< " != "
			<< "0x" << setfill('0') << setw(2) << static_cast<int>(e.calculated)
			<< endl;
		return -1;
	} catch (Record::unknown_type_exception) {
		cerr
			<< "ERROR: " << argv[0] << ": unknown record type"
			<< endl;
		return -1;
	} catch (Region::continuous_exception) {
		cerr
			<< "ERROR: " << argv[0] << ": region does not contain continuous data, not supported"
			<< endl;
		return -1;
	}

	// manipulate data

	if (options.erase_region.size()) {
		for (auto i = options.erase_region.begin(); i != options.erase_region.end(); ++i) {
			auto region = hex.find(*i);
			if (region == hex.end()) {
				cerr
					<< "warning: cannot erase region, base address "
					<< "0x" << setbase(16) << setfill('0') << setw(8) << *i
					<< setbase(10) << resetiosflags(ios::showbase)
					<< " not found"
					<< endl;
				continue;
			}
			hex.erase(region);
		}
	}

	if (options.move_region.size()) {
		for (auto i = options.move_region.begin(); i != options.move_region.end(); ++i) {
			auto region = hex.find(i->first);
			if (region == hex.end()) {
				cerr
					<< "warning: cannot move region, base address "
					<< "0x" << setbase(16) << setfill('0') << setw(8) << i->first
					<< setbase(10) << resetiosflags(ios::showbase)
					<< " not found"
					<< endl;
				continue;
			}
			if (hex.find(i->second) != hex.end()) {
				cerr
					<< "warning: cannot move region, destination base address "
					<< "0x" << setbase(16) << setfill('0') << setw(8) << i->second
					<< setbase(10) << resetiosflags(ios::showbase)
					<< " already exists"
					<< endl;
				continue;
			}
			region->move_base_address(i->second);
		}
	}

	// output results

	if (options.info) {
		print_info(cout, hex);
	} else if (options.dump) {
		hex.dump_data(cout, options.dump_width);
	} else if (options.ihex) {
		hex.dump_ihex(cout, options.ihex_width);
	}

	cin.rdbuf(cin_old);
	cout.rdbuf(cout_old);

	return 0;
}

