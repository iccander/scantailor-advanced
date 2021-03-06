// Copyright (C) 2019  Joseph Artsimovich <joseph.artsimovich@gmail.com>, 4lex4 <4lex49@zoho.com>
// Use of this source code is governed by the GNU GPLv3 license that can be found in the LICENSE file.

#ifndef SCANTAILOR_CORE_PROJECTWRITER_H_
#define SCANTAILOR_CORE_PROJECTWRITER_H_

#include <foundation/Hashes.h>

#include <QString>
#include <Qt>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index_container.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

#include "ImageId.h"
#include "OutputFileNameGenerator.h"
#include "PageId.h"
#include "PageSequence.h"
#include "SelectedPage.h"
#include "VirtualFunction.h"

class AbstractFilter;
class ProjectPages;
class PageInfo;
class QDomDocument;
class QDomElement;

class ProjectWriter {
  DECLARE_NON_COPYABLE(ProjectWriter)

 public:
  using FilterPtr = std::shared_ptr<AbstractFilter>;

  ProjectWriter(const std::shared_ptr<ProjectPages>& pageSequence,
                const SelectedPage& selectedPage,
                const OutputFileNameGenerator& outFileNameGen);

  ~ProjectWriter();

  bool write(const QString& filePath, const std::vector<FilterPtr>& filters) const;

  /**
   * \p out will be called like this: out(ImageId, numeric_image_id)
   */
  template <typename OutFunc>
  void enumImages(OutFunc out) const;

  /**
   * \p out will be called like this: out(LogicalPageId, numeric_page_id)
   */
  template <typename OutFunc>
  void enumPages(OutFunc out) const;

 private:
  struct Directory {
    QString path;
    int numericId;

    Directory(const QString& path, int numericId) : path(path), numericId(numericId) {}
  };

  struct File {
    QString path;
    int numericId;

    File(const QString& path, int numericId) : path(path), numericId(numericId) {}
  };

  struct Image {
    ImageId id;
    int numericId;
    int numSubPages;
    bool leftHalfRemoved;
    bool rightHalfRemoved;

    Image(const PageInfo& pageInfo, int numericId);
  };

  struct Page {
    PageId id;
    int numericId;

    Page(const PageId& id, int numericId) : id(id), numericId(numericId) {}
  };

  class Sequenced;

  using MetadataByImage = std::unordered_map<ImageId, ImageMetadata>;

  using Directories = boost::multi_index::multi_index_container<
      Directory,
      boost::multi_index::indexed_by<
          boost::multi_index::hashed_unique<boost::multi_index::member<Directory, QString, &Directory::path>,
                                            hashes::hash<QString>>,
          boost::multi_index::sequenced<boost::multi_index::tag<Sequenced>>>>;

  using Files = boost::multi_index::multi_index_container<
      File,
      boost::multi_index::indexed_by<
          boost::multi_index::hashed_unique<boost::multi_index::member<File, QString, &File::path>,
                                            hashes::hash<QString>>,
          boost::multi_index::sequenced<boost::multi_index::tag<Sequenced>>>>;

  using Images = boost::multi_index::multi_index_container<
      Image,
      boost::multi_index::indexed_by<
          boost::multi_index::hashed_unique<boost::multi_index::member<Image, ImageId, &Image::id>, std::hash<ImageId>>,
          boost::multi_index::sequenced<boost::multi_index::tag<Sequenced>>>>;

  using Pages = boost::multi_index::multi_index_container<
      Page,
      boost::multi_index::indexed_by<
          boost::multi_index::hashed_unique<boost::multi_index::member<Page, PageId, &Page::id>, std::hash<PageId>>,
          boost::multi_index::sequenced<boost::multi_index::tag<Sequenced>>>>;

  QDomElement processDirectories(QDomDocument& doc) const;

  QDomElement processFiles(QDomDocument& doc) const;

  QDomElement processImages(QDomDocument& doc) const;

  QDomElement processPages(QDomDocument& doc) const;

  void writeImageMetadata(QDomDocument& doc, QDomElement& imageEl, const ImageId& imageId) const;

  int dirId(const QString& dirPath) const;

  int fileId(const QString& filePath) const;

  QString packFilePath(const QString& filePath) const;

  int imageId(const ImageId& imageId) const;

  int pageId(const PageId& pageId) const;

  void enumImagesImpl(const VirtualFunction<void, const ImageId&, int>& out) const;

  void enumPagesImpl(const VirtualFunction<void, const PageId&, int>& out) const;

  PageSequence m_pageSequence;
  OutputFileNameGenerator m_outFileNameGen;
  SelectedPage m_selectedPage;
  Directories m_dirs;
  Files m_files;
  Images m_images;
  Pages m_pages;
  MetadataByImage m_metadataByImage;
  Qt::LayoutDirection m_layoutDirection;
};


template <typename Callable>
void ProjectWriter::enumImages(Callable out) const {
  enumImagesImpl(ProxyFunction<Callable, void, const ImageId&, int>(out));
}

template <typename Callable>
void ProjectWriter::enumPages(Callable out) const {
  enumPagesImpl(ProxyFunction<Callable, void, const PageId&, int>(out));
}

#endif  // ifndef SCANTAILOR_CORE_PROJECTWRITER_H_
