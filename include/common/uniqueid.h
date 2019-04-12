struct UniqueId
{
	virtual bool operator==(const std::unique_ptr<UniqueId>& other) const = 0;
	bool operator!=(const std::unique_ptr<UniqueId>& other) const
	{
		return !(*this == other);
	}
};