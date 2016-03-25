
#pragma once

class Updater {
public:

	Updater();
	~Updater();

	bool IsSupported() const;

	std::string GetStatus() const;

private:

	struct Impl;
	std::unique_ptr<Impl> mImpl;
};

