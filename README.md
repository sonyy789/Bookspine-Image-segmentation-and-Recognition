# Bookspine-Image-segmentation-and-Recognition
Bookspine Segmentation that divde a book Image from bookshelf Image, Bookspine Recognition is matching procedure Using feature extraction.
Book management system using computer vision.
This is sample version of that system.

Original Image
<img src="https://user-images.githubusercontent.com/16300242/50549848-8ca1da80-0ca7-11e9-95fd-8bd284631705.JPG" width="90%"></img>

Edge Detection Using binarization
<img src="https://user-images.githubusercontent.com/16300242/50549847-8ad81700-0ca7-11e9-85a0-b7f31a9374cd.JPG" width="90%"></img>
    many short lines between edges of book. >>> are detected by Hough Transform >> have to remove

Short line Filtered Using by morphology 
<img src="https://user-images.githubusercontent.com/16300242/50549846-890e5380-0ca7-11e9-8f3a-58c64f2f8e6b.JPG" width="90%"></img>

Short line Filtered Using harris corner and etc.
<img src="https://user-images.githubusercontent.com/16300242/50549845-86136300-0ca7-11e9-9d12-7e3f332d994f.JPG" width="90%"></img>

Segmentation Image
<img src="https://user-images.githubusercontent.com/16300242/50549843-81e74580-0ca7-11e9-88cd-6ec7bf34299d.JPG" width="90%"></img>

Segmented 
<img src="https://user-images.githubusercontent.com/16300242/50549841-801d8200-0ca7-11e9-9a0b-3ed39fb84c3f.JPG" width="10%"></img>

Matching Using SIFT, BF-matching
<img src="https://user-images.githubusercontent.com/16300242/50549836-5c5a3c00-0ca7-11e9-98ec-e677209043ca.JPG" width="90%"></img>

This is sample version. 
Compare between segmented Image.

In real Server, Compare segmented Image and real book Image from camera.
Plus, Stored pre-calculated Data(keypoints & descriptors) to database for using SIFT(higher matching rate.)
