/*
This file is part of Telegram Desktop,
the official desktop version of Telegram messaging app, see https://telegram.org

Telegram Desktop is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

It is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

In addition, as a special exception, the copyright holders give permission
to link the code of portions of this program with the OpenSSL library.

Full license: https://github.com/telegramdesktop/tdesktop/blob/master/LICENSE
Copyright (c) 2014-2015 John Preston, https://desktop.telegram.org
*/
#pragma once

#include <QtGui/QPixmap>

QImage imageBlur(QImage img);
void imageRound(QImage &img);

inline uint32 packInt(int32 a) {
	return (a < 0) ? uint32(int64(a) + 0x100000000LL) : uint32(a);
}
inline int32 unpackInt(uint32 a) {
	return (a > 0x7FFFFFFFU) ? int32(int64(a) - 0x100000000LL) : int32(a);
}
inline uint64 packUIntUInt(uint32 a, uint32 b) {
	return (uint64(a) << 32) | uint64(b);
}
inline uint64 packUIntInt(uint32 a, int32 b) {
	return packUIntUInt(a, packInt(b));
}
inline uint64 packIntUInt(int32 a, uint32 b) {
	return packUIntUInt(packInt(a), b);
}
inline uint64 packIntInt(int32 a, int32 b) {
	return packUIntUInt(packInt(a), packInt(b));
}
inline uint32 unpackUIntFirst(uint64 v) {
	return uint32(v >> 32);
}
inline int32 unpackIntFirst(uint64 v) {
	return unpackInt(unpackUIntFirst(v));
}
inline uint32 unpackUIntSecond(uint64 v) {
	return uint32(v & 0xFFFFFFFFULL);
}
inline int32 unpackIntSecond(uint64 v) {
	return unpackInt(unpackUIntSecond(v));
}

class StorageImageLocation {
public:
	StorageImageLocation() : _widthheight(0), _dclocal(0), _volume(0), _secret(0) {
	}
	StorageImageLocation(int32 width, int32 height, int32 dc, const uint64 &volume, int32 local, const uint64 &secret) : _widthheight(packIntInt(width, height)), _dclocal(packIntInt(dc, local)), _volume(volume), _secret(secret) {
	}
	StorageImageLocation(int32 width, int32 height, const MTPDfileLocation &location) : _widthheight(packIntInt(width, height)), _dclocal(packIntInt(location.vdc_id.v, location.vlocal_id.v)), _volume(location.vvolume_id.v), _secret(location.vsecret.v) {
	}
	bool isNull() const {
		return !_dclocal;
	}
	int32 width() const {
		return unpackIntFirst(_widthheight);
	}
	int32 height() const {
		return unpackIntSecond(_widthheight);
	}
	void setSize(int32 width, int32 height) {
		_widthheight = packIntInt(width, height);
	}
	int32 dc() const {
		return unpackIntFirst(_dclocal);
	}
	uint64 volume() const {
		return _volume;
	}
	int32 local() const {
		return unpackIntSecond(_dclocal);
	}
	uint64 secret() const {
		return _secret;
	}

private:
	uint64 _widthheight;
	uint64 _dclocal;
	uint64 _volume;
	uint64 _secret;

	friend inline bool operator==(const StorageImageLocation &a, const StorageImageLocation &b) {
		return (a._dclocal == b._dclocal) && (a._volume == b._volume) && (a._secret == b._secret);
	}

};

inline bool operator!=(const StorageImageLocation &a, const StorageImageLocation &b) {
	return !(a == b);
}

QPixmap imagePix(QImage img, int32 w, int32 h, bool smooth, bool blurred, bool rounded, int32 outerw, int32 outerh);

class HistoryItem;
class Image {
public:

	Image(QByteArray format = "PNG") : format(format), forgot(false) {
	}

	virtual void automaticLoad(const HistoryItem *item) { // auto load photo
	}

	virtual bool loaded() const {
		return true;
	}
	virtual bool loading() const {
		return false;
	}
	virtual bool displayLoading() const {
		return false;
	}
	virtual void cancel() {
	}
	virtual float64 progress() const {
		return 1;
	}
	virtual int32 loadOffset() const {
		return 0;
	}

	const QPixmap &pix(int32 w = 0, int32 h = 0) const;
	const QPixmap &pixRounded(int32 w = 0, int32 h = 0) const;
	const QPixmap &pixBlurred(int32 w = 0, int32 h = 0) const;
	const QPixmap &pixColored(const style::color &add, int32 w = 0, int32 h = 0) const;
	const QPixmap &pixBlurredColored(const style::color &add, int32 w = 0, int32 h = 0) const;
	const QPixmap &pixSingle(int32 w, int32 h, int32 outerw, int32 outerh) const;
	const QPixmap &pixBlurredSingle(int32 w, int32 h, int32 outerw, int32 outerh) const;
	QPixmap pixNoCache(int32 w = 0, int32 h = 0, bool smooth = false, bool blurred = false, bool rounded = false, int32 outerw = -1, int32 outerh = -1) const;
	QPixmap pixColoredNoCache(const style::color &add, int32 w = 0, int32 h = 0, bool smooth = false) const;
	QPixmap pixBlurredColoredNoCache(const style::color &add, int32 w, int32 h = 0) const;

	virtual int32 width() const = 0;
	virtual int32 height() const = 0;

	virtual void load(bool loadFirst = false, bool prior = true) {
	}
	virtual void loadEvenCancelled(bool loadFirst = false, bool prior = true) {
	}

	bool isNull() const;
	
	void forget() const;
	void restore() const;

	QByteArray savedFormat() const {
		return format;
	}
	QByteArray savedData() const {
		return saved;
	}

	virtual ~Image() {
		invalidateSizeCache();
	}

protected:

	virtual void checkload() const {
	}

	virtual const QPixmap &pixData() const = 0;
	virtual void doForget() const = 0;
	virtual void doRestore() const = 0;

	void invalidateSizeCache() const;

	mutable QByteArray saved, format;
	mutable bool forgot;

private:

	typedef QMap<uint64, QPixmap> Sizes;
	mutable Sizes _sizesCache;

};

class LocalImage : public Image {
public:

	LocalImage(const QString &file, QByteArray format = QByteArray());
	LocalImage(const QByteArray &filecontent, QByteArray format = QByteArray());
	LocalImage(const QPixmap &pixmap, QByteArray format = QByteArray());
	LocalImage(const QByteArray &filecontent, QByteArray format, const QPixmap &pixmap);

	int32 width() const;
	int32 height() const;

	~LocalImage();

protected:

	const QPixmap &pixData() const;
	void doForget() const {
		data = QPixmap();
	}
	void doRestore() const { 
		QBuffer buffer(&saved);
		QImageReader reader(&buffer, format);
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
		reader.setAutoTransform(true);
#endif
		data = QPixmap::fromImageReader(&reader, Qt::ColorOnly);
	}

private:

	mutable QPixmap data;
};

LocalImage *getImage(const QString &file, QByteArray format);
LocalImage *getImage(const QByteArray &filecontent, QByteArray format);
LocalImage *getImage(const QPixmap &pixmap, QByteArray format);
LocalImage *getImage(const QByteArray &filecontent, QByteArray format, const QPixmap &pixmap);

typedef QPair<uint64, uint64> StorageKey;
inline uint64 storageMix32To64(int32 a, int32 b) {
	return (uint64(*reinterpret_cast<uint32*>(&a)) << 32) | uint64(*reinterpret_cast<uint32*>(&b));
}
inline StorageKey storageKey(int32 dc, const uint64 &volume, int32 local) {
	return StorageKey(storageMix32To64(dc, local), volume);
}
inline StorageKey storageKey(const MTPDfileLocation &location) {
	return storageKey(location.vdc_id.v, location.vvolume_id.v, location.vlocal_id.v);
}
inline StorageKey storageKey(const StorageImageLocation &location) {
	return storageKey(location.dc(), location.volume(), location.local());
}

class StorageImage : public Image {
public:

	StorageImage(const StorageImageLocation &location, int32 size = 0);
	StorageImage(const StorageImageLocation &location, QByteArray &bytes);
	
	int32 width() const;
	int32 height() const;

	void automaticLoad(const HistoryItem *item); // auto load photo

	bool loaded() const;
	bool loading() const;
	bool displayLoading() const;
	void cancel();
	float64 progress() const;
	int32 loadOffset() const;

	void setData(QByteArray &bytes, const QByteArray &format = QByteArray());

	void load(bool loadFirst = false, bool prior = true);
	void loadEvenCancelled(bool loadFirst = false, bool prior = true);

	~StorageImage();

protected:

	const QPixmap &pixData() const;
	void checkload() const;
	void doForget() const {
		_data = QPixmap();
	}
	void doRestore() const { 
		QBuffer buffer(&saved);
		QImageReader reader(&buffer, format);
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
		reader.setAutoTransform(true);
#endif
		_data = QPixmap::fromImageReader(&reader, Qt::ColorOnly);
	}

private:
	mutable QPixmap _data;
	StorageImageLocation _location;
	int32 _size;
	mutable mtpFileLoader *_loader;

	bool amLoading() const {
		return _loader && _loader != CancelledFileLoader;
	}

};

StorageImage *getImage(const StorageImageLocation &location, int32 size = 0);
StorageImage *getImage(const StorageImageLocation &location, const QByteArray &bytes);
Image *getImage(int32 width, int32 height, const MTPFileLocation &location);

class ImagePtr : public ManagedPtr<Image> {
public:
	ImagePtr();
	ImagePtr(const QString &file, QByteArray format = QByteArray()) : Parent(getImage(file, format)) {
	}
	ImagePtr(const QByteArray &filecontent, QByteArray format = QByteArray()) : Parent(getImage(filecontent, format)) {
	}
	ImagePtr(const QByteArray &filecontent, QByteArray format, const QPixmap &pixmap) : Parent(getImage(filecontent, format, pixmap)) {
	}
	ImagePtr(const QPixmap &pixmap, QByteArray format) : Parent(getImage(pixmap, format)) {
	}
	ImagePtr(const StorageImageLocation &location, int32 size = 0) : Parent(getImage(location, size)) {
	}
	ImagePtr(const StorageImageLocation &location, const QByteArray &bytes) : Parent(getImage(location, bytes)) {
	}
	ImagePtr(int32 width, int32 height, const MTPFileLocation &location, ImagePtr def = ImagePtr());
};

void clearStorageImages();
void clearAllImages();
int64 imageCacheSize();

class PsFileBookmark;
class ReadAccessEnabler {
public:
	ReadAccessEnabler(const PsFileBookmark *bookmark);
	ReadAccessEnabler(const QSharedPointer<PsFileBookmark> &bookmark);
	bool failed() const {
		return _failed;
	}
	~ReadAccessEnabler();

private:
	const PsFileBookmark *_bookmark;
	bool _failed;

};

class FileLocation {
public:
	FileLocation(StorageFileType type, const QString &name);
	FileLocation() : size(0) {
	}

	bool check() const;
	const QString &name() const;
	void setBookmark(const QByteArray &bookmark);
	QByteArray bookmark() const;
	bool isEmpty() const {
		return name().isEmpty();
	}

	bool accessEnable() const;
	void accessDisable() const;

	StorageFileType type;
	QString fname;
	QDateTime modified;
	qint32 size;

private:
	QSharedPointer<PsFileBookmark> _bookmark;

};
inline bool operator==(const FileLocation &a, const FileLocation &b) {
	return a.type == b.type && a.name() == b.name() && a.modified == b.modified && a.size == b.size;
}
inline bool operator!=(const FileLocation &a, const FileLocation &b) {
	return !(a == b);
}

typedef QPair<uint64, uint64> MediaKey;
inline uint64 mediaMix32To64(int32 a, int32 b) {
	return (uint64(*reinterpret_cast<uint32*>(&a)) << 32) | uint64(*reinterpret_cast<uint32*>(&b));
}
inline MediaKey mediaKey(LocationType type, int32 dc, const uint64 &id) {
	return MediaKey(mediaMix32To64(type, dc), id);
}
inline StorageKey mediaKey(const MTPDfileLocation &location) {
	return storageKey(location.vdc_id.v, location.vvolume_id.v, location.vlocal_id.v);
}
