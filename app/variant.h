/*
 * Fedora Media Writer
 * Copyright (C) 2017 Martin Bříza <mbriza@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef VARIANT_H
#define VARIANT_H

#include <QObject>
#include <QList>
#include <QString>

class Progress;
class FileType;
class Architecture;

/**
 * @brief The Variant class
 *
 * The variant of the release version. Usually it represents different architectures. It's possible to differentiate netinst and dvd images here too.
 *
 * @property name the name of the release, generated from @ref arch and @ref board
 * @property board the name of supported hardware of the image
 * @property url the URL pointing to the image
 * @property image the path to the image on the drive
 * @property fileType the type of the image on the drive
 * @property size the size of the image in bytes
 * @property progress the progress object of the image - reports the progress of download
 * @property status status of the variant - if it's downloading, being written, etc.
 * @property statusString string representation of the @ref status
 * @property errorString a string better describing the current error @ref status of the variant
 */
class Variant final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(FileType *fileType READ fileType CONSTANT)

    Q_PROPERTY(QString file READ file NOTIFY fileChanged)
    Q_PROPERTY(QString fileName READ fileName CONSTANT)
    Q_PROPERTY(qreal size READ size NOTIFY sizeChanged)
    Q_PROPERTY(Progress* progress READ progress CONSTANT)

    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusString READ statusString NOTIFY statusChanged)
    Q_PROPERTY(QString errorString READ errorString WRITE setErrorString NOTIFY errorStringChanged)
public:
    Q_ENUMS(Type)
    enum Status {
        PREPARING = 0,
        DOWNLOADING,
        DOWNLOAD_RESUMING,
        DOWNLOAD_VERIFYING,
        READY,
        WRITING_NOT_POSSIBLE,
        WRITING,
        WRITE_VERIFYING,
        FINISHED,
        FAILED_DOWNLOAD,
        FAILED
    };
    Q_ENUMS(Status)
    const QStringList m_statusStrings {
        tr("Preparing"),
        tr("Downloading"),
        tr("Resuming download"),
        tr("Checking the download"),
        tr("Ready to write"),
        tr("Image file was saved to your downloads folder. Writing is not possible"),
        tr("Writing"),
        tr("Checking the written data"),
        tr("Finished!"),
        tr("The written data is corrupted"),
        tr("Download failed"),
        tr("Error")
    };

    Variant(QString url, Architecture *arch, FileType *fileType, QString board, const bool live, QObject *parent);

    static Variant *custom(const QString &path, QObject *parent);

    Q_INVOKABLE void setDelayedWrite(const bool value);

    Architecture *arch() const;
    FileType *fileType() const;
    QString name() const;
    QString board() const;
    bool live() const;

    QString url() const;
    QString file() const;
    QString fileName() const;
    qreal size() const;
    Progress *progress();

    Status status() const;
    QString statusString() const;
    void setStatus(Status s);
    QString errorString() const;
    void setErrorString(const QString &o);

    Q_INVOKABLE bool erase();

signals:
    void fileChanged();
    void statusChanged();
    void errorStringChanged();
    void sizeChanged();
    void cancelledDownload();

public slots:
    void download();
    void cancelDownload();
    void resetStatus();
    void onImageDownloadFinished();

private:
    const QString m_url;
    const QString m_fileName;
    const QString m_board;
    const bool m_live;
    QString m_file;
    Architecture *m_arch;
    FileType *m_fileType;
    qreal m_size = 0.0;
    Status m_status;
    QString m_error;
    bool delayedWrite;

    Progress *m_progress;

    void setSize(const qreal value);
};

#endif // VARIANT_H
